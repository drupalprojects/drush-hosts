<?php
class hostingService_http extends hostingService {
  public $service = 'http';
  public $ssl_enabled = FALSE;
}


class hostingService_http_public extends hostingService_http {
  protected $has_port = TRUE;
  protected $has_log_directory = TRUE;

  function default_port() {
    return 80;
  }

  function update() {
    parent::update();
  }
  
  public function context_options($task_type, $ref_type, &$task) {
    parent::context_options($task_type, $ref_type, $task);
  }


  public function validate(&$node, &$form) {
    parent::validate($node, $form);
    if (!empty($this->log_directory) && !is_dir($this->log_directory)) {
      form_set_error('log_directory', t("The log directory you specified does not exist."));
    }
  }

}

class hostingService_http_apache extends hostingService_http_public {
  public $type = 'apache';

  protected $has_restart_cmd = TRUE;

  function default_restart_cmd() {
    $command = '/usr/sbin/apachectl'; # a proper default for most of the world
    foreach (explode(':', $_SERVER['PATH']) as $path) {
      $options[] = "$path/apache2ctl";
      $options[] = "$path/apachectl";
    }
    # try to detect the apache restart command
    $options[] = '/usr/local/sbin/apachectl'; # freebsd
    $options[] = '/usr/sbin/apache2ctl'; # debian + apache2
    $options[] = $command;

    foreach ($options as $test) {
      if (is_executable($test)) {
        $command = $test;
        break;
      }
    }

    return "sudo $command graceful";
  }

  function default_log_directory() {
    $log_directory = '/var/log/apache2'; #debian/ubuntu
    $options[] = '/var/log/apache'; #centos?
    $options[] = '/var/log/httpd'; #redhat/centos ?
    $options[] = $log_directory;

    foreach ($options as $test) {
      if(is_dir($test)) {
        $log_directory = $test;
        break;
      }
    }
   return $log_directory;
  }
}
