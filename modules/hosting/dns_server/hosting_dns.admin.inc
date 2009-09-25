<?php
// $Id: hosting_dns.admin.inc,v 1.3 2009/04/16 01:46:52 adrian Exp $

// Admin configuration/UI hooks

function hosting_dns_admin() {
  $zone_count = db_result(db_query("SELECT COUNT(*) FROM {hosting_dns_soa}"));
  $output .= 'Aegir is managing '. $zone_count . ' zones.';

  $header = array('Name', 'Serial', 'Records', 'Active',  array('data' => t('Operations'), 'colspan' => 3));
  $result = db_query("SELECT z.nid, z.origin, z.serial, n.status FROM {hosting_dns_soa} z JOIN {node} n ON n.nid = z.nid ORDER BY origin");
  while ($row = db_fetch_array($result)) {
    $records = db_result(db_query("SELECT COUNT(*) FROM {hosting_dns_rr} WHERE nid = %d", $row['nid']));
    $rows[] = array(l(t($row['origin']), "node/". $row['nid']),
                        $row['serial'],
                        $records,
                        $row['active'],
                        l(t('records'), "node/". $row['nid'] . "/records"),
                        l(t('edit'), "node/". $row['nid'] . "/edit"),
                        l(t('delete'), "node/". $row['nid'] . "/delete")
                   );
  }
  $output .= theme('table', $header, $rows);
  return $output;
}

function hosting_dns_records($node) {
  $nid = $node->nid;
  $record_count = db_result(db_query("SELECT COUNT(*) FROM {hosting_dns_rr} WHERE nid=%d", $nid));
  $output = sprintf('The %s zone currently has %d records.', $zone->origin, $record_count);
  
  $header = array('Name', 'Type', 'Data', 'AUX', array('data' => t('Operations'), 'colspan' => 2));
  $result = db_query("SELECT rid, name, type, data, aux FROM {hosting_dns_rr} WHERE nid=%d ORDER BY
 name", $nid);
  while ($row = db_fetch_array($result)) {
    $rows[] = array(l(t($row['name']), "node/". $nid . "/records/" . $row['rid']
),
                        $row['type'],
                        $row['data'],
                        $row['aux'],
                        l(t('edit'), "node/". $nid . "/records/". $row['rid']),
                        l(t('delete'), "node/". $nid . "/records/". $row['rid'] 
. "/delete"),
                   );
  }
  $output .= theme('table', $header, $rows);
  return $output;
}

function hosting_dns_zone_delete_confirm($form_state, $nid) {
  $name = db_result(db_query("SELECT origin FROM {hosting_dns_soa} WHERE nid = %d", $nid));
  $form['nid'] = array('#type' => 'value', '#value' => $nid);
  $output = confirm_form($form,t('Are you sure you want to delete the zone %name (%nid)?', array('%name' => $name, '%nid' => $nid)), "admin/hosting/dns", "This action cannot be undone.", t('Delete'), t('Cancel'));
  return $output;
}

function hosting_dns_zone_delete_confirm_submit($form, &$form_state) {
  # remove zone from db (soa table)
  # schedule task to delete zone
  if (!hosting_dns_zone('delete', $form_state['values'])) {
    drupal_set_message("cannot delete zone");
  }
  $form_state['redirect'] = 'admin/hosting/dns';
}

function hosting_dns_record_form($form_state, $zone, $rid=0) {
  $form = array();
  if (!is_object($zone)) {
    $zone = node_load($zone);
  }
  $form['nid'] = array('#type' => 'value', '#value' => $zone->nid);

  if ($rid) { # Update an existing record
    $submit = t('Update Record');
    $record = (object) $zone->records[$rid];
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
    hosting_dns_rr('update', $form_state['values']['nid'], $form_state['values']);
# schedule a task to commit update
  } else {
    hosting_dns_rr('add', $form_state['values']['nid'], $form_state['values']);
# schedule a task to commit update
  }
  $form_state['redirect'] = 'node/'. $form_state['values']['nid'] .'/records';
}

function hosting_dns_record_delete_confirm($form_state, $node, $rid) {
  if (!is_object($node)) {
    $node = node_load($node);
  }
  $nid = $node->nid;
  $origin = db_result(db_query("SELECT origin FROM {hosting_dns_soa} WHERE nid = %d", $nid));
  $name = db_result(db_query("SELECT name FROM {hosting_dns_rr} WHERE nid = %d AND rid = %d", $nid,
 $rid));

  $form['nid'] = array('#type' => 'value', '#value' => $nid);
  $form['rid'] = array('#type' => 'value', '#value' => $rid);
  $output = confirm_form($form, t('Are you sure you want to delete the zone record "'. $name .'" from
 the ' . $origin . ' zone?'), "node/". $nid . "/records", "This action cannot be
 undone.", t('Delete'), t('Cancel'));
  return $output;
}

function hosting_dns_record_delete_confirm_submit($form, &$form_state) {
  # Schedule task to delete the record?
  hosting_dns_rr('delete', $form_state['values']['nid'], array('rid' => $form_state['values']['rid']));
  $form_state['redirect'] = 'node/'. $form_state['values']['nid'] .'/records';
}
