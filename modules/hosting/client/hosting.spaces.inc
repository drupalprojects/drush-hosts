<?php


/**
 * Implementation of hook_init().
 */
function hosting_client_init() {
  if (arg(0) === 'node' && is_numeric(arg(1))) {
    spaces_init_space('client', arg(1));
  }
}

/**
 * Implementation of hook_ctools_plugin_api().
 */
function hosting_client_ctools_plugin_api($module, $api) {
  if ($module == 'spaces' && $api == 'plugins') {
    return array('version' => 3);
  }
}

/**
 * Implementation of hook_spaces_plugins().
 */
function hosting_client_spaces_plugins() {
  $plugins = array();
  $plugins['space_client'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'hosting_client') .'/plugins',
      'file' => 'space_client.inc',
      'class' => 'space_client',
      'parent' => 'space_type',
    ),
  );
  return $plugins;
}

/**
 * Implementation of hook_spaces_registry().
 */
function hosting_client_spaces_registry() {
  return array(
    'types' => array(
      'client' => array(
        'title' => t('Client space'),
        'plugin' => 'space_client',
        'path' => 'node/%node',
      ),
    ),
  );
}

/**
 * Implementation of hook_user().
function hosting_client_user($op, &$edit, &$account, $category = NULL) {
  switch ($op) {
    case 'form':
      if ($category === 'account') {
        module_load_include('inc', 'spaces', 'spaces.admin');
        $presets = spaces_preset_load(NULL, 'client');
        if (count($presets) === 1) {
          $form['spaces_preset_client'] = array('#type' => 'value', '#value' => key($presets));
        }
        else if (count($presets) > 1) {
          $form = array();
          $form['spaces_preset'] = spaces_preset_form($presets, 'client', 'save');
          $form['spaces_preset']['#type'] = 'fieldset';
        }
        return $form;
      }
      break;
    case 'insert':
    case 'update':
      // Save preset values
      if (!empty($edit['spaces_preset_client'])) {
        $space = spaces_load('client', $account->uid);
        if ($space->activate()) {
          $space->controllers->variable->set('spaces_preset_client', $edit['spaces_preset_client']);
        }
      }
      break;
  }
}

 */

/**
 * Implementation of hook_form_alter() for spaces_features_form.
 * Disallow site_frontpage var from being customizable within a user space.
 */
function hosting_client_form_spaces_features_form_alter(&$form, &$form_state) {
  $space = spaces_get_space();
  if ($space && $space->type === 'client' && !empty($form['site_frontpage'])) {
    unset($form['site_frontpage']);
  }
}


/**
 * Implementation of hook_purl_provider().
 */
function hosting_client_purl_provider() {
  $items = array();
  $items["spaces_client"] = array(
    'name' => t('Client space'),
    'description' => t('Sets a space as the active space.'),
    'callback' => 'spaces_init_space',
    'callback arguments' => array('client'),
    'example' => 'my-space',
  );
  return $items;
}
