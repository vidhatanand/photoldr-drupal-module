<?php
/**
 * @file
 * Admin page callback for the photoldr_button module.
 */
function photoldr_button_admin_settings() {

  $options = array('no', 'yes');
  $form['photoldr_button_on_forms'] = array(
    '#type' => 'select',
    '#title' => t('Button on Node Forms'),
    '#options' => $options,
    '#default_value' => variable_get('photoldr_button_on_forms', 'no'),
    '#description' => t('Show the PhotoLDR app link on node add/edit forms.'),
  );


  return system_settings_form($form);
}
