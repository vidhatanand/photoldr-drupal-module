<?php
/**
 * @file
 * Post page for the photoldr module.
 *
 * /?q=photoldrpost.php
 *
 * Accepts images in POST and strings in POST and GET
 */


/**
 * This is a form page for posting from the iOS app.
 *
 * Use username and password in POST or GET authenticate.
 */
function photoldr_post_page() {
  global $language;
  $langcode = isset($langcode) ? $langcode : $language->language;
  $restrictedfields = array('pass', 'data', 'rdf_mapping', 'roles');

  // Give the user feedback.  Accumulate througouht process.
  $feed_user = "";

  // Decide on Get or Post  Post preffered.
  if (isset($_POST['username'])) {
    $data = $_POST;
  }
  elseif (isset($_GET['username'])) {
    $data = $_GET;
  }
  else {
    return;
  }
  $default_type = variable_get('photoldr_default_post_type', 'page');
  $ntype = isset($data['type']) ? drupal_strtolower(check_plain($data['type'])) : $default_type;

  // Authenticate the user.
  $uid = _photoldr_html_user_auth();
  if ($uid == FALSE) {
    drupal_access_denied();
    return;
  }
  else {
    $account = user_load($uid);

    $access = 'create ' . $ntype . ' content';

    if (user_access($access, $account)) {
      $usercreate = TRUE;
    }
    else {
      $usercreate = FALSE;
    }

    $access = 'edit any ' . $ntype . ' content';
    if (user_access($access, $account)) {
      $usereditany = TRUE;
    }
    else {
      $usereditany = FALSE;
    }

    $access = 'edit own ' . $ntype . ' content';
    if (user_access($access, $account)) {
      $usereditown = TRUE;
    }
    else {
      $usereditown = FALSE;
    }

    $access = 'delete any ' . $ntype . ' content';
    if (user_access($access, $account)) {
      $userdeleteany = TRUE;
    }
    else {
      $userdeleteany = FALSE;
    }

    $access = 'delete own ' . $ntype . ' content';
    if (user_access($access, $account)) {
      $userdeleteown = TRUE;
    }
    else {
      $userdeleteown = FALSE;
    }

    $access = 'administer nodes';
    if (user_access($access, $account)) {
      $nodeadmin = TRUE;
    }
    else {
      $nodeadmin = FALSE;
    }

    if (!($usercreate)) {
      drupal_access_denied();
      print "<!doctype html><html><head><meta charset=\"UTF-8\"><title>Access Denied</title></head><body>Access Denied</body></html>";
      return;
    }
  }

  // Debug post data.  set uid to -10 to disable.
  if ($uid == 1) {
    $testdata = $data;
    unset($testdata['password']);
    $handle = fopen("postdata.txt", "a");
    fwrite($handle, 'Data: \n' . print_r($testdata, TRUE) . '  ');
    fclose($handle);
  }

  $types = node_type_get_names();
  if (!isset($types[$ntype])) {
    watchdog('PhotoLDR',
      '%ntype is not a valid node type on this site.  <br /> %uri <br /> %host <br /> %addr',
      array(
        '%ntype' => $ntype,
        '%uri' => request_uri(),
        '%host' => $_SERVER['REMOTE_HOST'],
        '%addr' => ip_address(),
      )
      );
    return;
  }

  // Since we have a valid node type, decide if we are
  // modifying or creating a node.
  $nid = (isset($data['nid'])) ? check_plain($data['nid']) : 'new';

  // Load existing node by node id.
  if ($nid != 'new') {

    $node = node_load($nid);
    if (!$node) {
      // Could not load the nid, create a new node.
      $nid = 'new';
    }
    elseif ($node->type != $ntype) {
      // Node types do not match, so do not mangle
      // the node, create a new node.
      $nid = 'new';
    }

    if (($node->uid != $uid) && ($usereditany == FALSE)) {
      // Only the owner of the node can modify it.
      drupal_access_denied();
      print "<!doctype html><html><head><meta charset=\"UTF-8\"><title>Access Denied</title></head><body>Access Denied</body></html>";
      return;
    }
  }

  // Create blank node object in memory.
  if ($nid == 'new') {
    // Setup the node structure.
    $node = new stdClass();
    $node->nid = NULL;

    // Insert new data.
    $node->type = $ntype;
    node_object_prepare($node);
    $node->uid = $uid;
    $node->name = $account->name;
  }

  $field_types = field_info_field_types();
  $instances = field_info_instances('node', $node->type);
  $module = array();

  if ($nid == 'new') {
    // Building a blank node.
    foreach ($instances as $instance) {
      $field_name = $instance['field_name'];
      $field = field_info_field($field_name);
      // Skeleton node object with $node->field_name.
      $node->{$instance['field_name']} = "";
      $module[] = $field_types[$field['type']]['module'];
    }
  }

  // Get the default addresses from the user or Instance
  // if the user is not set.
  // Build node address field.
  if (isset($instances['field_address'])) {
    $node->field_address['und']['0'] = isset($account->field_address['und']['0']) ? $account->field_address['und']['0'] : $instances['field_address']['default_value'];
    unset($node->field_address['und']['0']['thoroughfare']);
    unset($node->field_address['und']['0']['premise']);
  }

  // Loop through $data array and fill in the node values.
  foreach ($data as $k => $v) {
    if (is_array($v)) {
      // If $v is an array, make it a string.
      $v = implode('; ', $v);
    }

    switch (drupal_strtolower($k)) {
      case 'status':
        $node->status = (isset($v)) ? check_plain($v) : $node->status;
        break;

      case 'image_overwrite':
        $image_overwrite = (isset($v)) ? check_plain($v) : FALSE;
        break;

      case 'title':
        $node->title = ($v) ? check_plain($v) : $node->title;
        break;

      case 'body':
        $node->body['und']['0']['value'] = isset($v) ? check_markup($v, 'full_html') : $node->body['und']['0']['value'];
        $node->body['und']['0']['safe_value'] = isset($v) ? check_markup($v, 'full_html') : $node->body['und']['0']['safe_value'];

        break;

      case 'street1':
        $node->field_address['und']['0']['thoroughfare'] = ($v) ? check_plain($v) : $node->field_address['und']['0']['thoroughfare'];
        break;

      case 'street2':
        $node->field_address['und']['0']['premise'] = ($v) ? check_plain($v) : $node->field_address['und']['0']['premise'];
        break;

      case 'city':
        $node->field_address['und']['0']['locality'] = ($v) ? check_plain($v) : $node->field_address['und']['0']['locality'];
        break;

      case 'state':
        $node->field_address['und']['0']['administrative_area'] = ($v) ? check_plain(drupal_strtoupper($v)) : $node->field_address['und']['0']['administrative_area'];
        break;

      case 'zip':
        $node->field_address['und']['0']['postal_code'] = ($v) ? check_plain($v) : $node->field_address['und']['0']['postal_code'];
        break;

      case 'country':
        $node->field_address['und']['0']['country'] = ($v) ? check_plain(drupal_strtoupper($v)) : $node->field_address['und']['0']['country'];
        break;

      case 'phone':
        if ($nid != 'new') {
          $node->field_phone['und']['0']['number'] = ($v) ? check_plain($v) : $node->field_phone['und']['0']['number'];
        }
        if ($nid == 'new') {
          $node->field_phone['und'][] = array('number' => check_plain($v));
        }
        break;

      case 'address':
        $useraddress = _post_widget_get_user_address($uid);
        $node->field_address['und'][] = $useraddress;
        break;

      case 'standalone':
        $node->field_standalone['und']['0']['value'] = check_plain($v);
        break;

      default:
        if (isset($node->{'field_' . $k})) {
          $node->{'field_' . $k}['und'] = check_plain($v);
        }

        // END switch case.
    }

    // END foreach $data.
  }

  // IMAGE handling. ----------------------------------------
  $images = array();
  $instance = field_info_instances('node', $ntype);
  $imagefieldnames = array();

  foreach ($instance as $k => $v) {
    if ($v['widget']['module'] == 'image') {
      $imagefieldnames[] = $v['field_name'];
    }

    // Date field handling  date("Y-m-d H:i", strtotime($exp_date)).
    if ($v['widget']['module'] == 'date') {
      $datefield = $v['field_name'];
      $node->{$datefield}['und']['0'] = array(
        'value' => isset($data[$datefield]) ? date("Y-m-d H:i", strtotime(check_plain($data[$datefield]))) : $node->{$datefield}['und']['0']['value'],
        'value2' => isset($data[$datefield . '_end']) ? date("Y-m-d H:i", strtotime(check_plain($data[$datefield . '_end']))) : $node->{$datefield}['und']['0']['value2'],
      );

      if ($nid == 'new') {
        $node->{$datefield}['und']['0'] = array(
          'timezone' => $account->timezone,
        );
      }

      // End date handling.
    }

  }

  if (count($imagefieldnames) > 1) {
    // There is more than on image field!!!
    $photoldr_interact_image_field = $imagefieldnames['0'];
  }
  if (count($imagefieldnames) == 1) {
    $photoldr_interact_image_field = $imagefieldnames['0'];
  }

  // Get the field name for the image field.
  if (isset($instances[$photoldr_interact_image_field])) {
    $field_name = $instances[$photoldr_interact_image_field]['field_name'];
  }

  if (isset($field_name)) {

    if (($nid == 'new') && (!isset($node->{$field_name}))) {
      // Create empty node field for image.
      $node->{$field_name} = '';
    }
    if ($image_overwrite != FALSE) {
      unset($node->{$field_name});
      $node->{$field_name} = '';
    }

    $imagefieldinfo = field_info_field($field_name);
    switch ($imagefieldinfo['cardinality']) {
      case FIELD_CARDINALITY_UNLIMITED:
        $max = '10';
        break;

      default:
        $max = $imagefieldinfo['cardinality'];

    }
    $images = _photoldr_process_images($ntype, $uid, $field_name);

    if (is_array($images)) {

      foreach ($images as $image) {
        // Final touches on the Image Field.
        $image['title'] = '';
        $node->{$field_name}['und'][] = $image;
        // $url = file_create_url($image['uri']);
      }

      // End ISARRAY IMAGES.
    }

    // END ISSET FIELDNAME.
  }

  node_save($node);

  // Debug post data.  st uid to -10 to disable.
  if ($uid == 1) {
    $testdata = $node;
    // unset($testdata['password']);
    $handle = fopen("postdata.txt", "a");
    fwrite($handle, 'Node: \n' . print_r($testdata, TRUE) . '  ');
    fclose($handle);
  }

  // This is an XML return.
  $xml_head = '<?xml version="1.0" encoding="utf-8"?><domains><domain>';
  $xml_foot = '</domain></domains>';

  if ($node->nid) {
    watchdog('PhotoLDR', 'Posted data for %nid as uid %uid.', array(
      '%nid' => $node->nid,
      '%uid' => $node->uid,
    ));
  }

  if ($node->nid) {
    $result = array();
    $result[] = $node;
    $pub = _photoldr_prep_node_for_xml($result, $pub, $account);
    $xml_body = _photoldr_nodes_to_xml($pub, $restrictedfields);
    drupal_add_http_header('Content-Type', 'application/xml');
    print $xml_head;
    print $xml_body;
    print $xml_foot;
  }
  else {
    // This is an HTML return.
    // print "<h1>ERROR - Node not saved.</h1>";
    // print $node_rendered;
    return "<h1>ERROR - Node not saved.</h1>";

  }

  return;
}


/**
 * Private function to process images in POST.
 *
 * $type is the node type to associate
 * $uid is the authenticate user uid
 * $field_name is the image field name for this node type/
 */
function _photoldr_process_images($type, $uid, $field_name) {
  $feed_user = "";
  if (!isset($_FILES)) {
    // No Files in _POST.
    return;
  }

  // Test _FILES for images.
  $count = 0;
  foreach ($_FILES as $k => $v) {
    if (preg_match('/imag*/i', $k)) {
      // Count Images in _POST.
      $count++;
    }
  }
  if ($count == 0) {
    // No Images in POST.
    return;
  }

  // Image Field Handling ************************************************
  $images = array();
  $instances = field_info_instances('node', $type);
  $instance = $instances[$field_name];
  $field = field_info_field($field_name);

  // Image Field Options **************************************************
  switch ($field['cardinality']) {
    case FIELD_CARDINALITY_UNLIMITED:
      $max = '9';
      break;

    default:
      $max = $field['cardinality'] - 1;

  }
  $uri = $instance['settings']['file_directory'];
  $max_size = $instance['settings']['max_filesize'];
  $extensions = $instance['settings']['file_extensions'];
  $max_res = $instance['settings']['max_resolution'];
  $min_res = $instance['settings']['min_resolution'];

  $validators = array(
    'file_validate_is_image' => array(),
    'file_validate_extensions' => array($extensions),
    'file_validate_size' => array($max_size * 1024 * 1024),
  );

  // Set and Prepare the file directory.
  $picture_directory = variable_get('file_default_scheme', 'public') . '://' . $uri . '/';
  file_prepare_directory($picture_directory, FILE_CREATE_DIRECTORY);

  $i = "0";
  foreach ($_FILES as $k => $v) {
    $src = new stdClass();
    $src->uri = $v['tmp_name'];
    $fuid = str_pad((int) $uid, 6, "0", STR_PAD_LEFT);
    $fdate = date('Y-m-d-Hi', REQUEST_TIME);
    $filename = $type . '-' . $fuid . '-' . $fdate . '-' . $v['name'];

    // Make sure extension and filename are safe.
    $filename = file_munge_filename($filename, $extensions, FALSE);
    $destination = file_stream_wrapper_uri_normalize($picture_directory . $filename);
    $file_copy = file_copy($src, $destination, FILE_EXISTS_REPLACE);

    // Make sure we have an image.
    $validimg = file_validate_is_image($file_copy);
    $info = image_get_info($file_copy->uri);

    if (empty($validimg)) {
      $file_copy->status = 1;
    }

    // The file will be purged if it has a status of 0.
    if ($file_copy->status == 0) {
      file_delete($file_copy);
      $feed_user .= 'Bad file ' . $v['name'] . ' removed.<br />';
    }
    else {
      $result_save = file_save($file_copy);
      // This collects the image objects to save in the node.
      $images[$i] = $result_save;
      $images[$i]->alt = $type . ' image';
      $images[$i]->title = '';
    }

    // Adhere to cardinality regardless of input.
    if ($i == $max) {
      break;
    }
    $i++;
  }

  $images_return = '';

  foreach ($images as $key => $imgobj) {
    file_validate_image_resolution($imgobj, $max_res, $min_res);
    photoldr_autorotate_image($imgobj);

    foreach ($imgobj as $obj => $value) {
      $images_return[$key][$obj] = $value;
    }
  }

  return $images_return;

}

/**
 * Private function to rotate images if possible.
 *
 * $imagefile is the image object.
 */
function photoldr_autorotate_image($imagefile) {

  if (!function_exists('exif_read_data')) {
    // exif_read_data  function does not exist.
    return;
  }
  $filename = file_create_url($imagefile->uri);
  $image = image_load($imagefile->uri);
  $exif = exif_read_data($filename);
  $ort = isset($exif['Orientation']) ? $exif['Orientation'] : 1;

  switch ($ort) {
    case 1:
      // Nothing.
      break;

    case 2:
      // Horizontal flip.
      // image_flip($image, 1);
      break;

    case 3:
      // 180 rotate left.
      image_rotate($image, 180);
      image_save($image);
      break;

    case 4:
      // Vertical flip.
      // image_flip($image, 2);
      break;

    case 5:
      // Vertical flip + 90 rotate right.
      // image_flip($image, 2);
      image_rotate($image, -90);
      image_save($image);
      break;

    case 6:
      // 90 rotate right.
      image_rotate($image, -90);
      image_save($image);
      break;

    case 7:
      // Horizontal flip + 90 rotate right.
      // image_flip($image,1);
      image_rotate($image, -90);
      image_save($image);
      break;

    case 8:
      // 90 rotate left.
      image_rotate($image, 90);
      image_save($image);
      break;

  }
}
