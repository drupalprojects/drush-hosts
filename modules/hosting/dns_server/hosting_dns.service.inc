<?php

class hostingService_DNS extends hostingService {
  protected $service = 'dns';
  protected $type = 'bind';

  public function load() {
    parent::load();
    $this->mergeData('SELECT default_ip, ns, mbox, refresh, retry, expire, minimum, ttl, xfer FROM {hosting_dns_server} WHERE vid = %d', $this->server->vid);

    $config_obj = db_fetch_object(db_query('SELECT vid,data FROM {hosting_dns_server_config} WHERE vid = %d', $this->server->vid));
    $this->setValues(unserialize($config_obj->data));
  }

  function insert() {
    parent::insert();
    if ($this->default_dns_server == 1) {
      variable_set('hosting_default_dns_server', $this->server->nid);
    }
    db_query("INSERT INTO {hosting_dns_server} (vid, nid, default_ip, ns, mbox, refresh, retry, expire, minimum, ttl, xfer) 
      VALUES (%d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')", 
      $this->server->vid, $this->server->nid, $this->default_ip, $this->ns, $this->mbox, $this->refresh, $this->retry, $this->expire, $this->minimum, $this->ttl, $this->xfer);

    # prepare the data and serialize as array
    # zonedata and named_conf
    # named_checkconf, named_checkzone, and rndc
    $data = array(
      'zonedata' => $this->zonedata,
      'named_conf' => $this->named_conf,
      'named_checkconf' => $this->named_checkconf,
      'named_checkzone' => $this->named_checkzone,
    'rndc' => $this->rndc,
    );
    db_query("INSERT INTO {hosting_dns_server_config} (vid,nid,data) VALUES (%d, %d, '%s')",
       $this->server->vid, $this->server->nid, serialize($data));
  }


  /**
   * Implementation of hook_validate().
   *
   * TODO: validate all the various DNS fields!
   * and also the settings and utilities!
   */
  function validate(&$node, &$form) {
  }

  public function form(&$form) {
    parent::form($form);
    $node = $form['#node'];

    $form['zonedata'] = array(
      '#type' => 'textfield',
      '#title' => t('Zone Data Path'),
      '#description' => t('Full path to bind zonefiles. Must NOT be writable by Apache.'),
      '#default_value' => $node->zonedata ? $node->zonedata : HOSTING_DEFAULT_CONFIG_PATH .'/named',
      '#size' => 40,
      '#maxlength' => 128,
    );
    $form['named_conf'] = array(
      '#type' => 'textfield',
      '#title' => t('Local Bind Config Filename'),
      '#description' => t('Configuration file for Aegir-generated zones. Resides in zone data path and 
  must be included via named.conf'),
      '#default_value' => $node->named_conf ? $node->named_conf : 'named.conf.drupal',
      '#size' => 40,
      '#maxlength' => 128,
    );

    $form['named_checkconf'] = array(
      '#type' => 'textfield',
      '#title' => t('Full path to named-checkconf'),
      '#default_value' => $node->named_checkconf ? $node->named_checkconf : '/usr/sbin/named-checkconf',
      '#size' => 40,
      '#maxlength' => 128,
    );
    $form['named_checkzone'] = array(
      '#type' => 'textfield',
      '#title' => t('Full path to named-checkzone'),
      '#default_value' => $node->named_checkzone ? $node->named_checkzone : '/usr/sbin/named-checkzone',
      '#size' => 40,
      '#maxlength' => 128,
    );
    $form['rndc'] = array(
      '#type' => 'textfield',
      '#title' => t('Full path to rndc'),
      '#default_value' => $node->rndc ? $node->rndc : '/usr/sbin/rndc',
      '#size' => 40,
      '#maxlength' => 128,
    );
    $form['default_ip'] = array(
      '#type' => 'textfield',
      '#title' => t('Default Server IP'),
      '#default_value' => $node->default_ip?$node->default_ip:'127.0.0.1',
      '#size' => 20,
      '#maxlength' => 15,
      '#description' => t('Use this IP as the default server for new zones.'),
    );
    $form['xfer'] = array(
      '#type' => 'textfield',
      '#title' => t('Zone Transfer Access List'),
      '#default_value' => $node->xfer,
      '#size' => 40,
      '#maxlength' => 255,
      '#description' => t('Comma-separated list of IP addresses allowed to transfer this zone via AXFR'),
    );
    $form['mbox'] = array(
      '#type' => 'textfield',
      '#title' => t('DNS Admin Email Address'),
      '#default_value' => $node->mbox,
      '#size' => 40,
      '#maxlength' => 128,
      '#description' => t('Ampersand will be converted to a period as per specification. This value will be entered into zonefiles.'),
    );
    $form['ns'] = array(
      '#type' => 'textarea',
      '#title' => t('Name Servers'),
      '#default_value' => $node->ns,
      '#size' => 40,
      '#maxlength' => 128,
      '#description' => t('Default DNS servers for zonefiles eg: ns1.example.com (1 per line)')
    );
    $form['refresh'] = array(
      '#type' => 'textfield',
      '#title' => t('Refresh'),
      '#default_value' => $node->refresh?$node->refresh:'7200',
      '#size' => 40,
      '#maxlength' => 128,
      '#description' => t('How often secondary servers should refresh this zone (default 2h)'),
    );
    $form['retry'] = array(
      '#type' => 'textfield',
      '#title' => t('Retry'),
      '#default_value' => $node->retry?$node->retry:'300',
      '#size' => 40, 
      '#maxlength' => 128,
      '#description' => t('How often secondary servers should retry in case of a problem (default 5m)'),
    );
    $form['expire'] = array(
      '#type' => 'textfield',
      '#title' => t('Expire'),
      '#default_value' => $node->expire?$node->expire:'604800',
      '#size' => 40,
      '#maxlength' => 128,
      '#description' => t('How long until the secondary servers expire the zone data (default 4w)'),
    );
    $form['minimum'] = array(
      '#type' => 'textfield',
      '#title' => t('Minimum TTL'),
      '#default_value' => $node->minimum?$node->minimum:'86400',
      '#size' => 40,
      '#maxlength' => 128,
      '#description' => t('Minimum caching time in case of failed lookups (default 1w)'),
    );
    $form['ttl'] = array(
      '#type' => 'textfield',
      '#title' => t('Time To Live'),
      '#default_value' => $node->ttl?$node->ttl:'86400',
      '#size' => 40,
      '#maxlength' => 128,
      '#description' => t('Default TTL for zone resource records without their own TTL value (default 1w)'),
    );
    
    if ($node->nid != HOSTING_DEFAULT_DNS_SERVER) {
      $form['default_dns_server'] = array(
        '#type' => 'checkbox',
        '#return_value' => 1,
        '#title' => t('Make default dns server'),
      );
    } else {
      $form['default_dns_server'] = array(
        '#type' => 'value', 
        '#value' => 1
      );
    }

  }

  function view(&$render) {
    parent::view($render);
    $render['engine'] = array(
      '#type' => 'fieldset',
      '#title' => t('DNS engine: ' . $this->type),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => -1,
    );
    if (user_access('administer DNS provisioning')) {
      $render['engine']['zonedata'] = array(
        '#type' => 'item',
        '#title' => t('Zone Data directory'),
        '#value' => $this->zonedata,
        '#weight' => -1,
      );
  
      $render['engine']['named_conf'] = array(
        '#type' => 'item',
        '#title' => t('Named config file'),
        '#value' => $this->named_conf,
      );
  
      $render['engine']['named_checkconf'] = array(
        '#type' => 'item',
        '#title' => t('Named config utility'),
        '#value' => $this->named_checkconf,
      );
      $render['engine']['named_checkzone'] = array(
        '#type' => 'item',
        '#title' => t('Named zone utility'),
        '#value' => $this->named_checkzone,
      );
      $render['engine']['rndc'] = array(
        '#type' => 'item',
        '#title' => t('Named RNDC utility'),
        '#value' => $this->rndc,
      );
    }
  
    $render['defaults'] = array(
      '#type' => 'fieldset',
      '#title' => t('Defaults'),
      '#collapsible' => TRUE,
      '#collapsed' => $teaser?TRUE:FALSE,
    );
    $render['defaults']['default_ip'] = array(
      '#type' => 'item',
      '#title' => t('Default Server IP'),
      '#value' => $this->default_ip,
    );
    $render['defaults']['ns'] = array(
      '#type' => 'item',
      '#title' => t('Default NS Records'),
      '#value' => preg_replace("/\n/", '<br />', $this->ns),
    );
    $render['defaults']['mbox'] = array(
      '#type' => 'item',
      '#title' => t('DNS Admin Email'),
      '#value' => $this->mbox,
    );
    $render['defaults']['refresh'] = array(
      '#type' => 'item',
      '#title' => t('Refresh'),
      '#value' => $this->refresh,
    );
    $render['defaults']['retry'] = array(
      '#type' => 'item',
      '#title' => t('Retry'),
      '#value' => $this->retry,
    );
    $render['defaults']['expire'] = array(
      '#type' => 'item',
      '#title' => t('Expire'),
      '#value' => $this->expire,
    );
    $render['defaults']['minimum'] = array(
      '#type' => 'item',
      '#title' => t('Minimum TTL'),
      '#value' => $this->minimum
    );
    $render['defaults']['ttl'] = array(
      '#type' => 'item',
      '#title' => t('TTL'),
      '#value' => $this->ttl,
    );
    $render['defaults']['xfer'] = array(
      '#type' => 'item',
      '#title' => t('Zone Transfer Access List'),
      '#value' => $this->xfer,
    );
  # This should show a table of all sites that use this DNS server to host their domain?
  # Needs entries in hosting_site table to work properly..
  #  $render['sites_view'] = array(
  #    '#type' => 'item',
  #    '#title' => t("Sites"),
  #    '#value' => hosting_site_list("dns_server", $this->nid), 
  #    '#weight' => 10,
  #  );

  }

  function delete() {
    parent::delete();
    db_query('DELETE FROM {hosting_dns_server} WHERE nid = %d', $this->server->nid);
    db_query('DELETE FROM {hosting_dns_server_config} WHERE nid = %d', $this->server->nid);
  }

  function delete_revision() {
    parent::delete_revision();
    db_query('DELETE FROM {hosting_dns_server} WHERE vid = %d', $this->server->vid);
    db_query('DELETE FROM {hosting_dns_server_config} WHERE vid = %d', $this->server->vid);
  }

  function update() {
    parent::update();
    db_query("UPDATE {hosting_dns_server} SET default_ip = '%s', ns = '%s', mbox = '%s', refresh = '%s', retry = '%s', expire = '%s', minimum = '%s', ttl = '%s', xfer = '%s' WHERE vid = %d", $this->default_ip, $this->ns, $this->mbox, $this->refresh, $this->retry, $this->expire, $this->minimum, $this->ttl, $this->xfer, $this->server->vid);

   $data = array(
     'zonedata' => $this->zonedata,
     'named_conf' => $this->named_conf,
     'named_checkconf' => $this->named_checkconf,
     'named_checkzone' => $this->named_checkzone,
     'rndc' => $this->rndc,
   );
   db_query("UPDATE {hosting_dns_server_config} SET data = '%s' WHERE vid = %d", serialize($data), $this->server->vid);
  }

  public function options($task_type, $ref_type, &$task) {
    $task->options['dns_id'] = $this->server->nid;
    $task->options['dns_host'] = $this->server->title;

   if ($task_type == 'verify' && $ref_type == 'platform') {
      foreach (array('default_ip', 'ns', 'mbox', 'refresh', 'retry', 'expire', 'minimum', 'ttl', 'xfer') as $param) {;
        $task->options[$param] = $this->$param;
      }
    }
  }
}

