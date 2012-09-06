<?php
// $Id$

/**
 * @file
 * Admin page callback for the photoldr_button module.
 */

function photoldr_button_admin_settings() {
  $helptxt = '<p><strong>PhotoLDR Button must have node types allowed.  Then users permissions must be set to allow users to create content, or the post will fail. </strong></p>';
  
  $nodenames = node_type_get_names();
  $defaultnodeslist = array('article', 'book', 'page', 'product_display');
  $form['photoldr_button_types'] = array(
    '#type' => 'select',
    '#multiple' => 'TRUE',
    '#title' => t('Nodes'),
    '#options' => $nodenames,
    '#size' =>  min(2, count($nodenames)),
    '#default_value' => variable_get('photoldr_button_types', $defaultnodeslist),
    '#description' => t('Select the types of nodes used by PhotoLDR Button.'),
  );

  // drupal_set_message('<pre>' . print_r($nodenames, true) . '</pre>');


  return system_settings_form($form);
}