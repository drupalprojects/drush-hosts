<?php

/**
 * @file
 * Manage entries in /etc/hosts.
 */

define('HOSTS_BEGIN_MARKER', "#### Drush Hosts entries begin ####");
define('HOSTS_END_MARKER', "#### Drush Hosts entries end ####");

/**
 * Implements hook_drush_command().
 */
function hosts_drush_command() {
  $items = array();

  $items['hosts'] = array(
    'description' => dt('List entries in /etc/hosts.'),
    'options' => array(
      'all' => array(
        'description' => dt('List all entries from /etc/hosts, not just those managed by drush-hosts.')
      ),
    ),
    'aliases' => array('hs'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  $items['hosts-add'] = array(
    'description' => dt('Add entries to /etc/hosts. (Requires sudo password)'),
    'options' => array(
      'ip' => array(
        'description' => dt('The IP address of the server or site.')
      ),
      'fqdn' => array(
        'description' => dt('A comma-delimited list of domain names.'),
        'required' => TRUE,
      ),
    ),
    'examples' => array(
      'drush hsa --ip=192.168.22.13 --fqdn=foo.com,bar.net' => dt('Add an entry to /etc/hosts, so that foo.com and bar.net both point to the IP address 192.168.22.13.'),
    ),
    'strict-option-handling' => TRUE,
    'allow-additional-options' => TRUE,
    'aliases' => array('hsa'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );


  $items['hosts-remove'] = array(
    'description' => dt('Remove entries from /etc/hosts. (Requires sudo password)'),
    'options' => array(
      'ip' => array(
        'description' => dt('The IP address of the server or site.')
      ),
      'fqdn' => array(
        'description' => dt('A comma-delimited list of domain names.'),
        'required' => TRUE,
      ),
      'purge' => array(
        'description' => dt('Remove all entries in /etc/hosts made by Drush Hosts.')
      ),
    ),
    'examples' => array(
      'drush hsr --ip=192.168.22.13 --fqdn=foo.com,bar.net --remove' => dt('Remove the domains "foo.com" and "bar.net" from the entry in /etc/hosts for IP address 192.168.22.13.'),
      'drush hsr --ip=192.168.22.13 --remove' => dt('Remove the entire entry in /etc/hosts for IP address 192.168.22.13.'),
    ),
    'strict-option-handling' => TRUE,
    'allow-additional-options' => TRUE,
    'aliases' => array('hsr'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  return $items;
}

/**
 * Add an entry to /etc/hosts.
 */
function drush_hosts() {

  $hosts = _drush_hosts_get_hosts();

  // Extract our entries
  $entries =  _drush_hosts_get_entries($hosts);

  if (drush_get_option('all', FALSE)) {
    drush_print(implode("\n", $hosts['file']));
  }
  else {
    drush_print(implode("\n", $entries));
  }

}


/**
 * Add an entry to /etc/hosts.
 */
function drush_hosts_add() {
  $ip_addr = _drush_hosts_get_valid_ip();
  $domains = _drush_hosts_get_valid_domains(TRUE);

  // This command must be run as root, if not already try to run it through sudo.
  if (posix_getuid() > 0) {
    _drush_hosts_run_as_root('hosts-add');
  }
  else {

    $hosts = _drush_hosts_get_hosts();

    // Extract our entries
    $entries =  _drush_hosts_get_entries($hosts);

    // Find the IP address
    $ip_line = _drush_hosts_get_line_by_ip($entries, $ip_addr);
    // Add an IP address and/or domain
    if (!$ip_line) {
      $ip_line = count($entries);
      $entries[$ip_line] = $ip_addr . "   ";
    }
    // TODO: don't add duplicate domains
    foreach ($domains as $domain) {
      $entries[$ip_line] = $entries[$ip_line] . ' ' . $domain;
    }
    // Write the updated hosts file
    $hosts = _drush_hosts_update_hosts_entries($hosts, $entries);
    $result = _drush_hosts_set_hosts($hosts);

    return $result;
  }
}


/**
 * Add an entry to /etc/hosts.
 */
function drush_hosts_remove() {
  $ip_addr = _drush_hosts_get_valid_ip();
  $domains = _drush_hosts_get_valid_domains();

  // This command must be run as root, if not already try to run it through sudo.
  if (posix_getuid() > 0) {
    _drush_hosts_run_as_root('hosts-remove');
  }
  else {

    $hosts = _drush_hosts_get_hosts();

    // Handle --purge option
    if ($purge = drush_get_option('purge', FALSE)) {
      if (!drush_confirm('Remove all Drush Hosts entries from /etc/hosts?')) {
        drush_user_abort();
        return;
      }
      array_splice($hosts['file'], $hosts['begin'], ($hosts['end'] - $hosts['begin']) + 1, NULL);
      _drush_hosts_set_hosts($hosts);
      return;
    }

    // Extract our entries
    $entries =  _drush_hosts_get_entries($hosts);

    // Find the IP address
    $ip_line = _drush_hosts_get_line_by_ip($entries, $ip_addr);
    // Remove a line or domain
    if (!$ip_line) {
      drush_set_error('ERROR: Cannot find IP address ' . $ip_addr . ' in /etc/hosts.');
      return;
    }
    elseif (count($domains) > 0) {
      foreach ($domains as $domain) {
        if (strpos($entries[$ip_line], $domain) !== FALSE) {
          $entries[$ip_line] = str_replace(' ' . $domain, '', $entries[$ip_line]);
        }
        else {
          drush_set_error('ERROR: Cannot find FQDN "' . $domain . '" in /etc/hosts at the line for ' . $ip_addr . '.');
          return;
        }
      }
      // If there are no domains left on a line, remove it
      if (trim($entries[$ip_line]) == $ip_addr) {
        unset($entries[$ip_line]);
      }
    }
    else {
      unset($entries[$ip_line]);
    }

    // Write the updated hosts file
    $hosts = _drush_hosts_update_hosts_entries($hosts, $entries);
    $result = _drush_hosts_set_hosts($hosts);

    return $result;
  }
}

/**
 * Get all entries in /etc/hosts.
 */
function _drush_hosts_get_hosts() {
  $hosts = array();

  $hosts['file'] = file('/etc/hosts', FILE_IGNORE_NEW_LINES);

    // Find markers that delineate our block of entries
    foreach ($hosts['file'] as $count => $line) {
      if (strpos($line, HOSTS_BEGIN_MARKER) !== FALSE) {
        $hosts['begin'] = $count;
      }
      if (strpos($line, HOSTS_END_MARKER) !== FALSE) {
        $hosts['end'] = $count;
      }
    }
    // Add markers to end of file if none were found
    if (!isset($hosts['begin']) && !isset($hosts['end'])) {
      $hosts['begin'] = count($hosts['file']);
      $hosts['file'][] = HOSTS_BEGIN_MARKER;
      $hosts['end'] = count($hosts['file']);
      $hosts['file'][] = HOSTS_END_MARKER;
    }

    // Sanity check
    if (!($hosts['end'] > $hosts['begin'])) {
      drush_set_error('ERROR: Error parsing hosts file.');
      return FALSE;
    }

  return $hosts;
}

/**
 * Write to /etc/hosts
 */
function _drush_hosts_set_hosts($hosts) {
  $output = implode("\n", $hosts['file']) . "\n";
  $result = file_put_contents('/etc/hosts', $output);
  return $result;
}

/**
 * Run the command as root, prompting for a password if required
 */
function _drush_hosts_run_as_root($command) {
    $home = getenv('HOME');
    $options_array = drush_redispatch_get_options();
    $options_string = '';
    foreach ($options_array as $option => $value) {
      $options_string .= ' --' . $option . '=' . $value;
    }
    drush_shell_exec_interactive("sudo env HOME=$home drush " . $command . ' ' . $options_string);
}

/**
 * Return just the entries we're managing
 */
function _drush_hosts_get_entries($hosts) {
  return array_slice($hosts['file'], $hosts['begin'] + 1, ($hosts['end'] - $hosts['begin']) - 1);
}


function _drush_hosts_get_line_by_ip($entries, $ip_addr) {
  foreach ($entries as $count => $line) {
    if (strpos($line, $ip_addr) !== FALSE) {
      return $count;
    }
  }
  return FALSE;
}

function _drush_hosts_update_hosts_entries($hosts, $entries) {
  array_splice($hosts['file'], $hosts['begin'] + 1, ($hosts['end'] - $hosts['begin']) - 1, $entries);
  return $hosts;
}

function _drush_hosts_get_valid_ip() {
  if (!($ip_addr = drush_get_option('ip'))) {
    drush_set_error('ERROR: Could not find an IP address. Try specifying one with the "--ip" option, or using a VM alias.');
    exit(1);
  }

  if(filter_var($ip_addr, FILTER_VALIDATE_IP)) {
    // it's valid
    return $ip_addr;
  }
  else {
    // it's not valid
    drush_set_error("ERROR: Invalid IP address supplied: $ip_addr");
    exit(1);
  }

}

function _drush_hosts_get_valid_domains($required = FALSE) {

  $domains = drush_get_option_list('fqdn');

  if (is_null($domains[0])) {
    if ($required) {
      drush_set_error('ERROR: Could not find any domains. Try specifing one or more with the "--fqdn" option.');
      exit(1);
    }
    else {
      return;
    }
  }
  else {
    foreach ($domains as $domain) {
      if(!filter_var('http://' . $domain, FILTER_VALIDATE_URL)) {
        // it's not valid
        drush_set_error("ERROR: Invalid FQDN supplied: $domain");
        exit(1);
      }
    }
    // they're all valid
    return $domains;
  }

}

