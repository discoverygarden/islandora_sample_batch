<?php

/**
 * @file
 * Drush command to run the preprocessor.
 */

/**
 * Implements hook_drush_command().
 */
function sample_batch_drush_command() {
  return array(
    'sample_batch_preprocess' => array(
      'aliases' => array('sb-p'),
      'description' => dt('Sample batch preprocessor; scans a directory and turns every file into a Fedora object with the binary object content model'),
      // Believe it or not, this is required. Batches can be called from a
      // context that does not require the module to be enabled.
      'drupal dependencies' => array('sample_batch'),
      // A few options here. We need the path to the files to preprocess, and
      // some information about what to do with them.
      'options' => array(
        'path' => array(
          'description' => dt('The path to the files to preprocess; should be an absolute path with no aliases'),
          'required' => TRUE,
          'example-value' => '/path/to/some/files',
        ),
        'namespace' => array(
          'description' => dt('The namespace to give new objects; defaults to "islandora"'),
          'required' => FALSE,
          'example-value' => 'some-namespace',
        ),
        'parent' => array(
          'description' => dt('PID of the parent collection; defaults to "islandora:root"'),
          'required' => FALSE,
          'example-value' => 'some:pid',
        ),
      ),
    ),
  );
}

/**
 * Validation for the batch preprocessor.
 */
function drush_sample_batch_preprocess_validate() {
  // Validate that the path is real.
  $path = drush_get_option('path');
  if (!is_dir($path)) {
    drush_set_error('sample_batch_preprocess',
      dt('The path @path could not be resolved as a directory.', array(
        '@path' => $path,
      )));
  }
  // Validate that we can actually use this namespace.
  $namespace = drush_get_option('namespace', 'islandora');
  module_load_include('inc', 'islandora', 'includes/utilities');
  if (!islandora_is_valid_namespace($namespace)) {
    drush_set_error('sample_batch_preprocess',
      dt('The namespace @ns is not a valid namespace.', array(
        '@ns' => $namespace,
      )));
  }
  // Validate that the given parent is a real object.
  $parent = drush_get_option('parent', 'islandora:root');
  if (!islandora_object_load($parent)) {
    drush_set_error('sample_batch_preprocess',
      dt('Parent collection @parent does not exist.', array(
        '@parent' => $parent,
      )));
  }
}

/**
 * Callback for the batch preprocessor.
 */
function drush_sample_batch_preprocess() {
  // Take our parameters from the Drush options to pass on to the preprocessor.
  $params = array(
    'path' => drush_get_option('path'),
    'namespace' => drush_get_option('namespace', 'islandora'),
    'parent' => drush_get_option('parent', 'islandora:root'),
  );
  // XXX: Always reset islandora_get_tuque_connection before getting a new one
  // in Drush; just a quirk of how it works.
  drupal_static_reset('islandora_get_tuque_connection');
  $connection = islandora_get_tuque_connection();

  // Generate the preprocessor and send it off to batch.
  $preprocessor = new SampleBatchPreprocessor($connection, $params);
  islandora_batch_handle_preprocessor($preprocessor);
  // Dump out the set ID number.
  drush_print(dt('Set ID: @id', array('@id' => $preprocessor->getSetId())));
}
