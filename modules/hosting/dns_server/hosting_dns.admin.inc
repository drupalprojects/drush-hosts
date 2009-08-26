<?php
// $Id: hosting_dns.admin.inc,v 1.3 2009/04/16 01:46:52 adrian Exp $

// Admin configuration/UI hooks

/**
 * Implementation of hook_menu().
 **/
function hosting_dns_menu() {
# provide engine-specific settings here, depending on the types of the DNS nodes in the system..
    $items['admin/hosting/dns'] = array(
      'title' => t('DNS'),
      'description' => t('Manually administer DNS zones and records'),
      'page callback' => 'hosting_dns_admin',
      'access arguments' => array('administer DNS provisioning'),
      'type' => MENU_LOCAL_TASK
    );
    $items['admin/hosting/dns/list'] = array(
      'title' => t('List Zones'),
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -1,
    );
    $items['admin/hosting/dns/configure'] = array(
      'title' => t('Configure'),
      'type' => MENU_LOCAL_TASK,
      'description' => t('Configure DNS Provisioning'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('hosting_dns_settings'),
      'access arguments' => array('administer DNS provisioning'),
      'weight' => 10,
    );


    $items['admin/hosting/dns/add'] = array(
      'title' => t('Add Zone'),
      'description' => t('Add a new DNS zone (domain) to the system'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('hosting_dns_zone_form'),
      'access arguments' => array('access administration pages'),
      'type' => MENU_LOCAL_TASK,
    );

    $items['admin/hosting/dns/%/edit'] = array(
      'title' => t('Edit Zone'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('hosting_dns_zone_form', 3),
      'access arguments' => array('access administration pages'),
      'type' => MENU_CALLBACK,
    );
    $items['admin/hosting/dns/%/delete'] = array(
      'title' => t('Delete Zone'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('hosting_dns_zone_delete_confirm', 3),
      'access arguments' => array('access administration pages'),
      'type' => MENU_CALLBACK,
    );

    $items['admin/hosting/dns/%/records'] = array(
      'title' => t('Zone Records'),
      'page callback' => 'hosting_dns_records',
      'page arguments' => array(3),
      'access arguments' => array('access administration pages'),
      'type' => MENU_CALLBACK,
    );
    $items['admin/hosting/dns/%/records/list'] = array(
      'title' => t('List Zone Records'),
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'access arguments' => array('access administration pages'),
      'weight' => -10,
    );
    $items['admin/hosting/dns/%/records/add'] = array(
      'title' => t('Add Zone Record'),
      'description' => t('Add a new DNS zone record (subdomain) to the system'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('hosting_dns_record_form', 3), 
      'access arguments' => array('access administration pages'),
      'type' => MENU_LOCAL_TASK,
    );
    $items['admin/hosting/dns/%/records/%'] = array(
      'title' => t('Edit Zone Record'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('hosting_dns_record_form', 3, 5),
      'type' => MENU_CALLBACK,
      'access arguments' => array('access administration pages'),
    );

    $items['admin/hosting/dns/%/records/%/delete'] = array(
      'title' => t('Delete Zone Record'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('hosting_dns_record_delete_confirm', 3, 5),
      'access arguments' => array('access administration pages'),
      'type' => MENU_CALLBACK,
    );
  return $items;
}

/**
 * Menu callback: general settings for DNS provisioning
 *
 * Allow the admin to define a set of TLDs which are valid for this platform. These are used when spl
itting
 **/
function hosting_dns_settings() {
  $form['hosting_dns_tlds'] = array(
    '#type' => 'textarea',
    '#title' => t('Supported Top Level Domains'),
    '#description' => t('TLDs which this Aegir site can install. Used to determine the host and zone 
parts of the FQDN when provisioning sites. Enter one TLD (.com, .net, .co.uk, .net.au) per line.'),
    '#default_value' => variable_get('hosting_dns_tlds', ''),
  );
  return system_settings_form($form);
}

function hosting_dns_admin() {
  $zone_count = db_result(db_query("SELECT COUNT(*) FROM {hosting_dns_soa}"));
  $output .= 'Aegir is managing '. $zone_count . ' zones.';

  $header = array('Name', 'Serial', 'Records', 'Active',  array('data' => t('Operations'), 'colspan' 
=> 3));
  $result = db_query("SELECT zid, origin, serial, active FROM {hosting_dns_soa} ORDER BY origin");
  while ($row = db_fetch_array($result)) {
    $records = db_result(db_query("SELECT COUNT(*) FROM {hosting_dns_rr} WHERE zid = %d", $row['zid
']));
    $rows[] = array(l(t($row['origin']), "admin/hosting/dns/". $row['zid'] ."/records"),
                        $row['serial'],
                        $records,
                        $row['active'],
                        l(t('records'), "admin/hosting/dns/". $row['zid'] . "/records"),
                        l(t('edit'), "admin/hosting/dns/". $row['zid'] . "/edit"),
                        l(t('delete'), "admin/hosting/dns/". $row['zid'] . "/delete")
                   );
  }
  $output .= theme('table', $header, $rows);
  return $output;
}

function hosting_dns_records($zid) {
  if (!$zone = hosting_dns_status('zone', $zid)) {
    drupal_set_message(t("Cannot load DNS zone %zid", array("%zid" => $zid)), "error");
  }
  $record_count = db_result(db_query("SELECT COUNT(*) FROM {hosting_dns_rr} WHERE zid=%d", $zid));
  $output = sprintf('The %s zone currently has %d records.', $zone->origin, $record_count);
  
  $header = array('Name', 'Type', 'Data', 'AUX', array('data' => t('Operations'), 'colspan' => 2));
  $result = db_query("SELECT rid, name, type, data, aux FROM {hosting_dns_rr} WHERE zid=%d ORDER BY
 name", $zid);
  while ($row = db_fetch_array($result)) {
    $rows[] = array(l(t($row['name']), "admin/hosting/dns/". $zid . "/records/" . $row['rid']
),
                        $row['type'],
                        $row['data'],
                        $row['aux'],
                        l(t('edit'), "admin/hosting/dns/". $zid . "/records/". $row['rid']),
                        l(t('delete'), "admin/hosting/dns/". $zid . "/records/". $row['rid'] 
. "/delete"),
                   );
  }
  $output .= theme('table', $header, $rows);
  return $output;
}

function hosting_dns_zone_form($form_state, $zid = 0) {
  $fields = array('origin', 'ns1', 'ns2', 'mbox', 'serial', 'refresh', 'retry', 'expire', 'minimum', 
'ttl', 'active', 'xfer');
  $titles = array(t('Domain Name'), t('Primary NS'), t('Secondary NS'), t('Admin Email'), t('Serial')
, t('Refresh'), t('Retry'), t('Expire'), t('Minimum TTL'), t('TTL'), t('Active'), t('XFER Access'));
  $form = array();

  if ($zid) {
    # load up the zone, and prefill the form with its defaults
    $submit = t('Update Zone');
    $zone = hosting_dns_status('zone', array('zid' => $zid));
    $form['zid'] = array(
      '#type' => 'value',
      '#value' => $zone->zid,
    );
  } else {
    # create a 'default' zone object, with all the right fields set to defaults..
    $submit = t('Add Zone');
    $zone = new stdClass();
    foreach ($fields as $key) {
      $zone->$key = variable_get('hosting_dns_'.$key,NULL);
    }
    $zone->active = TRUE;
    $zone->serial = date('Ymd') . '01';
  }

  foreach ($fields as $i => $key) {
    $form[$key] = array(
      '#type' => ($key == 'active')?'checkbox':'textfield',
      '#title' => $titles[$i],
      '#default_value' => $zone->$key,
    );
  }

  $form['active']['#weight'] = -10;
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $submit,
  );
  return $form;
}


function hosting_dns_zone_form_validate($form_id, $form_values) {

  if (!$form_values['zid']) { # Create a new zone
    # Make sure it doesn't already exist
  }

  # Make sure all fields have valid values..

}

function hosting_dns_zone_form_submit($form, &$form_state) {
# update db (soa table)
  if ($form_state['values']['zid']) { # Update an existing zone
    hosting_dns_zone('update', $form_state['values']);   
# schedule task
  } else {                   # Create a new zone
    hosting_dns_zone('add', $form_state['values']);
# schedule task
  } 
  $form_state['redirect'] = 'admin/hosting/dns';
}

function hosting_dns_zone_delete_confirm($form_state, $zid) {
  $name = db_result(db_query("SELECT origin FROM {hosting_dns_soa} WHERE zid = %d", $zid));
  $form['zid'] = array('#type' => 'value', '#value' => $zid);
  $output = confirm_form($form,t('Are you sure you want to delete the zone %name (%zid)?', array('%name' => $name, '%zid' => $zid)), "admin/hosting/dns", "This action cannot be undone.", t('Delete'), t('Cancel'));
  return $output;
}

function hosting_dns_zone_delete_confirm_submit($form, &$form_state) {
  # remove zone from db (soa table)
  # schedule task to delete zone
  #XXX: this won't work: provision_dns_zone('delete', array('zid' => $form_values['zid']));
  if (!hosting_dns_zone('delete', $form_state['values'])) {
    drupal_set_message("cannot delete zone");
  }
  $form_state['redirect'] = 'admin/hosting/dns';
}

function hosting_dns_record_form($form_state, $zid, $rid=0) {
  $form = array();
  $form['zid'] = array('#type' => 'value', '#value' => $zid);

  if ($rid) { # Update an existing record
    $submit = t('Update Record');
    $record = hosting_dns_status('rr', array('zid' => $zid, 'rid' => $rid));
    $form['rid'] = array('#type' => 'value', '#value' => $rid);
  } else {
    $submit = t('Add Zone Record');
    $record = new stdClass();
    $record->name = '';
    $record->type = 'A';
    $record->data = '';
    $record->aux = 0;
    $record->ttl = '86400';
  }

  $form['name']  = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('The name of the record (subdomain)'),
    '#default_value' => $record->name,
  );
  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#description' => t('The type of record'),
    '#default_value' => $record->type,
    '#options' => array('A' => 'A', 'AAAA' => 'AAAA', 'ALIAS' => 'ALIAS', 'CNAME' => 'CNAME',
                        'HINFO' => 'HINFO', 'MX' => 'MX', 'NAPTR' => 'NAPTR', 'NS' => 'NS',
                        'PTR' => 'PTR', 'RP' => 'RP', 'SRV' => 'SRV', 'TXT' => 'TXT'),
  );
  $form['data'] = array(
    '#type' => 'textfield',
    '#title' => t('Data'),
    '#description' => t('The data where this record points to'),
    '#default_value' => $record->data,
  );
  $form['aux'] = array(
    '#type' => 'weight',
    '#title' => t('AUX/PRI'),
    '#description' => t('Auxiliary or Priority setting for this record'),
    '#default_value' => $record->aux,
  );
  $form['ttl'] = array(
    '#type' => 'textfield',
    '#title' => t('TTL'),
    '#description' => t('Time to Live setting for this record (numeric)'),
    '#default_value' => $record->ttl,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $submit,
  );

  return $form;
}

function hosting_dns_record_form_validate($form_id, $form_values) {
  # TODO: validate the form values!
}

function hosting_dns_record_form_submit($form, &$form_state) {
  if ($form_state['values']['rid']) {  # Update an existing zone
    hosting_dns_rr('update', $form_state['values']['zid'], $form_state['values']);
# schedule a task to commit update
  } else {
    hosting_dns_rr('add', $form_state['values']['zid'], $form_state['values']);
# schedule a task to commit update
  }
  $form_state['redirect'] = 'admin/hosting/dns/'. $form_state['values']['zid'] .'/records';
}

function hosting_dns_record_delete_confirm($form_state, $zid, $rid) {
  $origin = db_result(db_query("SELECT origin FROM {hosting_dns_soa} WHERE zid = %d", $zid));
  $name = db_result(db_query("SELECT name FROM {hosting_dns_rr} WHERE zid = %d AND rid = %d", $zid,
 $rid));

  $form['zid'] = array('#type' => 'value', '#value' => $zid);
  $form['rid'] = array('#type' => 'value', '#value' => $rid);
  $output = confirm_form($form, t('Are you sure you want to delete the zone record "'. $name .'" from
 the ' . $origin . ' zone?'), "admin/hosting/dns/". $zid . "/records", "This action cannot be
 undone.", t('Delete'), t('Cancel'));
  return $output;
}

function hosting_dns_record_delete_confirm_submit($form, &$form_state) {
  # Schedule task to delete the record?
  hosting_dns_rr('delete', $form_state['values']['zid'], array('rid' => $form_state['values']['rid']));
  $form_state['redirect'] = 'admin/hosting/dns/'. $form_state['values']['zid'] .'/records';
}
