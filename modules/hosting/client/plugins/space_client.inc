<?php

/**
 * User integration for Spaces. 
 */
class space_client extends space_type {

  /**
   * Override of load().
   * Load the user account object for this space and bail if it can't be loaded.
   */
  function load() {
    parent::load();
    $node = node_load(array('nid' => $this->id));
    if ($node->type == 'client') {
      $this->client = $node;
      return TRUE;
    }
    if ($node->type == 'site') {
      $this->client = node_load($node->client);
      return TRUE;
    }
    return FALSE;
  }

  function router($op, $object) {
    parent::router($op, $object);
    switch ($op) {
      case 'init':
        return;
      case 'node':
        $node = $object;
        if ($node->type == 'client') {
          if (isset($node->nid) && (!$this->active || ($this->active && $this->id != $node->nid)) && $space = spaces_load('client', $node->nid)) {
            $space->activate();
            return;
          }
        }
        elseif (($node->type == 'site') && isset($node->nid)) {
          if ($this->active && ($this->id == $node->client)) {
            return;
          }
          spaces_load('client', $node->client)->activate();
          return;
        }
        break;
    }
  }

  /**
   * Override of deactivate().
   * We don't have too many good options here except to leave the user/x path.
   * If the site frontpage has also been set to a user/x path, we just need
   * to fail miserably.
   */
  function deactivate() {
    if (strpos(variable_get('site_frontpage', 'node'), 'user') !== 0) {
      drupal_goto('<front>');
    }
    return;
  }

  /**
   * Override of title().
   */
  function title() {
    return $this->client->title;
  }

  function access_space($account = NULL) {
    global $user;
    $account = isset($account) ? $account : $user;
    return (in_array($this->id, array_keys( (array) $account->client_id)));
  }

  /**
   * Override of access_admin().
   */
  function access_admin($account = NULL) {
    if ($this->id != $this->client->nid) {
      return FALSE;
    }
    global $user;
    $account = isset($account) ? $account : $user;
    return (in_array($this->id, array_keys( (array) $account->client_id)) && user_access('edit own client')) || parent::access_admin($account);
  }
}
