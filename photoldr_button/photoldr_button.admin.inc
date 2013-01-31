<?php
/**
 * @file
 * Admin page callback for the photoldr_button module.
 */

/**
 * Builds and returns the PhotoLDR Button admin settings form.
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
  $form['photoldr_smart_app_banner'] = array(
    '#type' => 'select',
    '#title' => t('Smart App Banner'),
    '#options' => $options,
    '#default_value' => variable_get('photoldr_smart_app_banner', 'yes'),
    '#description' => t('Show the PhotoLDR Smart App Banner on this site.') . "  " . l('More Info', "https://developer.apple.com/library/safari/#documentation/AppleApplications/Reference/SafariWebContent/PromotingAppswithAppBanners/PromotingAppswithAppBanners.html"),
  );

  return system_settings_form($form);
}
