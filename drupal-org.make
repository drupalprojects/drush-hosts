core = 7.x
api = 2

; this makefile will fetch the Aegir distribution from drupal.org
;
; core aegir modules
; do *not* pin version number for Aegir core modules to make sure
; we fetch the latest release for those
projects[hosting][type] = "module"
projects[eldir][type] = "theme"

; contrib aegir modules
;projects[hosting_platform_pathauto][version] = "2.1"

; Contrib modules
projects[admin_menu][version] = "3.0-rc4"
projects[openidadmin][version] = "1.0"
projects[overlay_paths][version] = "1.3"
projects[views][version] = "3.7"
projects[ctools][version] = "1.3"
projects[views_bulk_operations][version] = "3.2"
projects[entity][version] = "1.3"
projects[js][version] = "1.0"

; Drop the js.php file from the 'High-performance JavaScript callback handler'
; project to the platform root. See: http://drupal.org/project/js
libraries[js][download][type] = "file"
libraries[js][download][url] = "http://drupalcode.org/project/js.git/blob_plain/HEAD:/js.php"
;libraries[js][download][url] = "http://drupalcode.org/project/js.git/blob/refs/tags/7.x-1.0:/js.php"
libraries[js][download][filename] = "../../../../js.php"

