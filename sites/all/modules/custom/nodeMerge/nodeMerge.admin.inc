<?php
include 'ChromePhp.php';
function nodeAnzeige_form($form){
    drupal_flush_all_caches();
    $option = array();
    if( isset($_SESSION['nid'])) {
        $nid = $_SESSION['nid'];
        // wenn nid's enthalten erstelle checkboxesOptionen
        if ($nid != null) {
            $epubCard = node_load_multiple($nid);
            foreach ($epubCard as $n) {
                $option[$n->title] = $n->title;
            }
        }
    }
    // wenn session nid als key enthält und nid nicht leer ist führe aus
    if( isset($_SESSION['nid']) and !empty($_SESSION['nid']) ) {
        $form['check'] = array(
            '#title' => 'Artikel auswahl',
            '#type' => 'checkboxes',
            '#description' => t('Wählen sie Artikel zum Löschen aus'),
            '#options' => $option,
        );
        $form['textfield'] = array(
          '#type' => 'textfield',
          '#title' => 'Titel',
          '#required' => TRUE,
        );
        $form['beschreibung'] = array(
          '#type' => 'textfield',
          '#title' => 'Beschreibung',
          '#required' => TRUE,
        );

        $form['submit']['delete'] = array(
            '#type' => 'submit',
            '#value' => 'delete articel',
            '#submit' => array('nodeAnzeige_delete')
        );
        $form['submit']['download'] = array(
            '#type' => 'submit',
            '#value' => 'download articel',
            '#submit' => array('download')
        );



    } else {
        $form['some_text'] = array(
            '#markup' => '<p><b>Sie haben keine Artikel aus gewählt !</p>'
        );
    }
    dpm($_SESSION);
    return $form;

}


function download($form){
    $_SESSION['title'] = $form['textfield']['#value'];
    $_SESSION['beschreibung'] = $form['beschreibung']['#value'];


    drupal_goto($path = 'admin/nodeMerge/download');

}

function nodeMerge(){

    error_reporting(E_ALL | E_STRICT);
    ini_set('error_reporting', E_ALL | E_STRICT);
    ini_set('display_errors', 1);

        $content_start =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        . "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
        . "    \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
        . "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
        . "<head>"
        . "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
        . "<link rel=\"stylesheet\" type=\"text/css\" href=\"styles.css\" />\n"
        . "<title>Test Book</title>\n"
        . "</head>\n"
        . "<body>\n";

        $bookEnd = "</body>\n</html>\n";


    include_once("EPub.php");

    $book = new EPub(); // no argumetns gives us the default ePub 2, lang=en and dir="ltr"

    // Title and Identifier are mandatory!
    $book->setTitle($_SESSION['beschreibung']);
    $book->setIdentifier("http://test.de", EPub::IDENTIFIER_URI); // Could also be the ISBN number, prefered for published books, or a UUID.
    $book->setLanguage("de"); // Not needed, but included for the example, Language is mandatory, but EPub defaults to "en". Use RFC3066 Language codes, such as "en", "da", "fr" etc.
    $book->setDescription("Epub export von InDekoNavi");
    //$book->setAuthor("John Doe Johnson", "Johnson, John Doe");
    $book->setPublisher("InDekoNavi", "http://InDekoNavi.com/");
    $book->setDate(time());
    $book->setRights("Copyright and licence information specific for the book."); // As this is generated, this _could_ contain the name or licence information of the user who purchased the book, if needed. If this is used that way, the identifier must also be made unique for the book.
    //$book->setSourceURL("http://JohnJaneDoePublications.com/books/TestBookSimple.html");

// A book need styling, in this case we use static text, but it could have been a file.
    $cssData = "body {\n  margin-left: .5em;\n  margin-right: .5em;\n  text-align: justify;\n}\n\np {\n  font-family: serif;\n  font-size: 10pt;\n  text-align: justify;\n  text-indent: 1em;\n  margin-top: 0px;\n  margin-bottom: 1ex;\n}\n\nh1, h2 {\n  font-family: sans-serif;\n  font-style: italic;\n  text-align: center;\n  background-color: #6b879c;\n  color: white;\n  width: 100%;\n}\n\nh1 {\n    margin-bottom: 2px;\n}\n\nh2 {\n    margin-top: -2px;\n    margin-bottom: 2px;\n}\n";
    $book->addCSSFile("styles.css", "css1", $cssData);

    $cover = $content_start . "<h1>Artikel von IndekoNavi</h1>\n<h2>erstellt am " .date('d.M.Y')."</h2>\n" . $bookEnd;
    $book->addChapter("Notices", "Cover.html", $cover);

    // array mit node ids
    $nid = $_SESSION['nid'];
    foreach ($nid as $nodeId){
        $nodeFull = getNodeInDatabase($nodeId);
        $epub = $content_start. '<h2>'.$nodeFull['title']. '</h2> <br  />' .$nodeFull['body'] .$bookEnd;
        $book->addChapter($nodeFull['title'], $nodeFull['title']  , $epub, true);
    }
    //epub an user senden
    $book->finalize();
    $book->sendBook($_SESSION['title']);

    //löscht die variable nid in der session
    unset($_SESSION['nid']);

    drupal_flush_all_caches();


}

//button delete()
function nodeAnzeige_delete($form, &$form_state){


    $sessionNid = $_SESSION['nid'];
    $node = node_load_multiple($sessionNid);
    $help = array();
    foreach ($node as $item){
     $help[$item->title] = $item->nid;
    }

    //Prüfe Checkbox
    foreach ($form_state['values']['check'] as $check){
        // wenn der wert nicht null ist (angeklickt bzw soll gelöscht werden)
        if (!empty($check)){
            //suche den titel der nid im $sessionNid und lösche den wert im $sessionNid
            if(($key = array_search($help[$check], $sessionNid)) !== false) {
                unset($sessionNid[$key]);

            }
        }
    }
    // ersetze das NidArray im SessionObjekt mit dem aktuellen
    $_SESSION['nid'] = $sessionNid;
    drupal_set_message("Node wurde gelöscht");
    drupal_flush_all_caches();

}
function getNodeInDatabase($nid){
    $title = db_query('SELECT n.title FROM {node} n WHERE n.nid = '. $nid )->fetchField();
    $body = db_query('SELECT n.body_value FROM {field_data_body} n WHERE n.entity_id = '.$nid )->fetchField();

    $node['title'] = $title;
    $node['body'] = nl2br($body);

    return $node;
};

function console_log( $data ){
    echo '<script>';
    echo 'console.log('. json_encode( $data ) .')';
    echo '</script>';
}
?>