<?php
// $Id: hosting_db_server.drush.inc,v 1.2 2009/05/07 22:04:10 adrian Exp $
/**
 * Implementation of hook_provision_args
 * Used to pass along configuration details to the backend provisioning tasks
 **/
function drush_hosting_dns_pre_hosting_task($task) {
  $task =& drush_get_context('HOSTING_TASK');
  if ($task->ref->type == 'platform' && $task->task_type == 'verify') {
    $dns = node_load($task->ref->dns_server);
    drush_log("loaded dns server info from platform: " . var_export($dns));
    $task->options['dns_id'] = $dns->nid;
    # $task->options['dns_zone'] = 
    $task->options['dns_ip'] = $dns->default_ip;
    $task->options['dns_expire'] = $dns->expire;
    $task->options['dns_retry'] = $dns->retry;
    $task->options['dns_minimum'] = $dns->minimum;
    $task->options['dns_ttl'] = $dns->ttl;
    $task->options['dns_xfer'] = $dns->xfer;
    $task->options['dns_email'] = $dns->email;
    $task->options['dns_ns1'] = $dns->ns1;
    $task->options['dns_ns2'] = $dns->ns2;

    # Engine specific stuff..
    $task->options['dns_zonedata'] = $dns->zonedata;
    $task->options['dns_named_conf'] = $dns->named_conf;
    $task->options['dns_named_checkconf'] = $dns->named_checkconf;
    $task->options['dns_named_checkzone'] = $dns->named_checkzone;
    $task->options['dns_named_rndc'] = $dns->named_rndc;
  } elseif ($task->ref->type == 'zone') {
    $zone = $task->ref;
    $task->args[0] = $task->task_type;
    // pass the whole zone node
    $task->options['zone'] = $zone;
  }
}

