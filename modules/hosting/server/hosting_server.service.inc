<?php


class hostingService {

  public $server;

  protected $has_restart_cmd = FALSE;
  protected $has_port = FALSE;
  protected $has_data_dir = FALSE;

  function __construct($node, $values = null) {
    self::save($node);
    self::setValues($values);
  }

  public function has_port() {
    return $this->has_port;
  }

  // Is this req'd? There isn't one for has_restart_cmd()...
  public function has_data_dir() {
    return $this->has_data_dir;
  }

  public function load() {
    $this->mergeData("SELECT port, restart_cmd, available, data_dir FROM {hosting_service} WHERE vid = %d AND type = '%s'", $this->server->vid, $this->type);
  }

  protected function mergeData($query) {
    $args = func_get_args();
    $result = db_fetch_array(call_user_func_array('db_query', $args));
    $this->setValues($result);
  }

  public function setValues($values = null) {
    if (is_array($values)) {
      foreach ($values as $key => $value) {
        $this->{$key} = $value;
      }
    }
  }

  public function save($node) {
    $this->server = new stdClass();
    $this->server->nid = $node->nid;
    $this->server->vid = $node->vid;
    $this->server->title = $node->title;
  }



  public function default_restart_cmd() {
    return '';
  }

  public function default_port() {
    return 0;
  }

  public function default_data_dir() {
    return '[platform_root]/sites/[uri]';
  }

  public function insert() {
    db_query("INSERT INTO {hosting_service} (nid, vid, service, type, port, restart_cmd, available, data_dir) VALUES (%d, %d, '%s', '%s', %d, '%s', %d, '%s')",
      $this->server->nid, $this->server->vid, $this->service, $this->type, $this->port, $this->restart_cmd, $this->available, $this->data_dir);
  }

  public function update() {
    $this->delete_revision();
    $this->insert();
  }
  
  public function delete() {
    db_query("DELETE FROM {hosting_service} WHERE service = '%s' AND nid=%d", $this->service, $this->server->nid);
  }

  public function delete_revision() {
    db_query("DELETE FROM {hosting_service} WHERE service = '%s' AND vid=%d", $this->service, $this->server->vid);
  }

  public function form(&$form) {
    if ($this->has_restart_cmd) {
      $form['restart_cmd'] = array(
          '#type' => 'textfield',
          '#title' => t('Restart command'),
          '#required' => $this->available,
          '#description' => t('The command to run to restart this service.'),
          '#default_value' => (!is_null($this->restart_cmd)) ? $this->restart_cmd : $this->default_restart_cmd(),
          '#size' => 40,
          '#maxlength' => 255,
          '#weight' => -7,
      );
    }
    else {
      $form['restart_cmd'] = array('#type' => 'value', '#value' => null);
    }
 

    if ($this->has_port) {
     $form['port'] = array(
        '#type' => 'textfield',
        '#title' => t('Port'),
        '#required' => $this->available,
        '#size' => 40,
        '#default_value' => ($this->port) ? $this->port : $this->default_port(),
        '#description' => t("The port that this service is listening on."),
        '#maxlength' => 255,
        '#weight' => -8,
      );
    }
    else {
      $form['port'] = array(
        '#type' => 'value',
        '#value' => '0',
      );
    }

    if ($this->has_data_dir) {
      $form['data_dir'] = array(
        '#type' => 'textfield',
        '#title' => t('Data directory'),
        '#required' => FALSE,
        '#size' => 40,
        '#default_value' => ($this->data_dir) ? $this->data_dir : $this->default_data_dir(),
      '#description' => t("The directory in which to install sites. Defaults to: '[platform_root]/sites/[uri]'.<br />
        Available tokens include:<br />
         &nbsp;&nbsp;[uri]           => Name of the site (e.g, www.example.com)<br />
         &nbsp;&nbsp;[platform_root] => Path to platforms (e.g., /var/aegir/platforms)<br />
         &nbsp;&nbsp;[platform_path] => Path to the platform (e.g., /var/aegir/platforms/drupal-7.15)<br />
         &nbsp;&nbsp;[client_name]   => Name of the client (e.g., 'admin')<br />
         &nbsp;&nbsp;[aegir_root]    => Root of the Aegir install (e.g., /var/aegir)<br />
         &nbsp;&nbsp;[clients_path]  => Path to clients directory (e.g., /var/aegir/clients)<br />
        "),
        '#maxlength' => 255,
      );
    }
 
  }

  public function view(&$render) {
    if ($this->has_data_dir) {
      $render['data_dir'] = array(
        '#type' => 'item',
        '#title' => t('Data directory'),
        '#value' => filter_xss($this->data_dir),
      );
    }

    if ($this->has_restart_cmd) {
      $render['restart_cmd'] = array(
        '#type' => 'item',
        '#title' => t('Restart command'),
        '#value' => filter_xss($this->restart_cmd),
      );

    } 
   
    if ($this->has_port) {
      $render['port'] = array(
        '#type' => 'item',
        '#title' => t('Port'),
        '#value' => filter_xss($this->port),
      );
    }

  }



  public function validate(&$node, &$form) {
    if ($this->has_port) {
      if ((int) $this->port <= 0) {
        form_set_error('port', t("The port you specify must be a number."));
      }
    }
    #TODO: Perhaps validate that our symlink source is a valid path?
  }

  public function context_options($task_type, $ref_type, &$task) {
    $task->context_options[$this->service . '_service_type'] = $this->type;

    if ($this->has_restart_cmd) {
      $task->context_options[$this->service . '_restart_cmd'] = $this->restart_cmd;
    }

    if ($this->has_port) {
      $task->context_options[$this->service . '_port'] = $this->port;
    }

    if ($this->has_data_dir) {
      $task->context_options[$this->service . '_data_dir'] = $this->data_dir;
    }

  }

  public function context_import($context) {
    $this->available = 1;

    if ($this->has_port) {
      $this->port = $context->{$this->service . '_port'};
    }

    if ($this->has_restart_cmd) {
      $this->restart_cmd = $context->{$this->service . '_restart_cmd'};
    }

    // Is order important here? has_port and has_restart_cmd are reversed from their usual order...
    if ($this->has_data_dir) {
      $this->data_dir = $context->{$this->service . '_data_dir'};
    }


  }

}

