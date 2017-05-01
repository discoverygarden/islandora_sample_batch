<?php

/**
 * @file
 * Sample batch preprocessor.
 */

/**
 * Sample batch preprocessor class.
 */
class SampleBatchPreprocessor extends IslandoraBatchPreprocessor {

  /**
   * Gets the correct content model for a filename.
   *
   * @TODO: This mapping should be fleshed out with all the content models and
   * mimetypes the batch intends to generate.
   */
  protected function getContentModel($file) {
    $map = array(
      'application/pdf' => 'islandora:sp_pdf',
      'audio/wav' => 'islandora:sp-audioCModel',
    );
    $mime = file_get_mimetype($file);
    return isset($map[$mime]) ? $map[$mime] : 'islandora:binaryObjectCModel';
  }

  /**
   * Really, all we need is this.
   *
   * This function should generate batch objects extended from
   * IslandoraBatchObject, and add them to the database using
   * $this->addToDatabase().
   *
   * This will serialize each batch object so that it's ready for processing
   * later.
   */
  public function preprocess() {
    // Get all the files in the given directory.
    $files = scandir($this->parameters['path']);
    // Remove '.' and '..'.
    $files = array_diff($files, array('.', '..'));

    // Scan through the files and generate a batch object for each one.
    foreach ($files as $file) {
      $object_params = array(
        // Pass on the absolute path to the file, so we can get its contents.
        'file' => $this->parameters['path'] . '/' . $file,
        'namespace' => $this->parameters['namespace'],
        'parent' => $this->parameters['parent'],
        'cmodel' => $this->getContentModel($file),
      );
      // Generate a new object ...
      $object = new SampleBatchPreprocessorObject($this->connection, $object_params);
      // ... and then add it to the database.
      $this->addToDatabase($object, $object->getResources(), $this->parameters['parent']);
    }
  }
}

/**
 * Sample batch preprocessor object.
 */
class SampleBatchPreprocessorObject extends IslandoraBatchObject {

  // Array of parameters we need to process this object, including a 'file',
  // a 'namespace', and a 'parent'.
  protected $params;

  /**
   * Constructor function.
   */
  public function __construct(IslandoraTuque $connection, $params) {
    // Get the next identifier for the given namespace so we can generate an
    // object by PID.
    $pid = $connection->repository->getNextIdentifier($params['namespace']);
    parent::__construct($pid, $connection->repository);
    // Store the parameters for later processing.
    $this->params = $params;
  }

  /**
   * Gets resources for this object. Required, but unused here.
   */
  public function getResources() {
    // No-op.
    return array();
  }

  /**
   * Required; processes this object.
   *
   * Standard practice is to call out to other methods that process individual
   * parts of the object. Here, we need an OBJ and some relationships.
   */
  public function batchProcess() {
    // Giving this a label from the file (we don't really have any other label
    // source to go off of).
    $this->label = $this->params['file'];
    // Standard processing sruff.
    $this->addOBJ();
    $this->addRelationships();
    // There are a few of these constants referring to batch states; once we're
    // done processing, pass back the __DONE state.
    return ISLANDORA_BATCH_STATE__DONE;
  }

  /**
   * Adds the OBJ datastream to the object.
   */
  public function addOBJ() {
    if (!isset($this['OBJ'])) {
      // If the datastream doesn't exist, build it from scratch.
      $obj = $this->constructDatastream('OBJ', 'M');
      $obj->label = "Archival File";
      $obj->mimetype = file_get_mimetype($this->params['file']);
    }
    else {
      $obj = $this['OBJ'];
    }
    // Set the content from the file we had.
    $obj->setContentFromFile($this->params['file']);
    // If the datastream doesn't exist, this will add it to the object. Will do
    // nothing otherwise.
    $this->ingestDatastream($obj);
  }

  /**
   * Required; adds relationships to the object.
   *
   * This includes the content model, and the parent.
   */
  public function addRelationships() {
    // Content models can simply be added using the 'models' parameter of the
    // object itself. Can be an array of strings, or a single string.
    $this->models = $this->params['cmodel'];
    // Parents are slightly more difficult; we're going to manually set this
    // using the object's 'relationship' object.
    $this->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $this->params['parent']);
  }

}
