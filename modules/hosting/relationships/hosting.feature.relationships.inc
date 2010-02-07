<?php

function hosting_relationships_hosting_feature() {
  $features['relationships'] = array(
    'title' => t('Relationships'),
    'description' => t('Establish relationships between elements in Aegir.'),
    'status' => HOSTING_FEATURE_ENABLED,
    'module' => 'hosting_relationships',
    );
  return $features;
}

