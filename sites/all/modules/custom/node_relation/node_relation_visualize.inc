<?php
/**
 * Created by PhpStorm.
 * User: julien
 * Date: 17.02.18
 * Time: 18:41
 */

function node_relation_visualize() {



    $data = node_relation_create();
    //dsm("HAllo");

    return network_draw($data);
}

function network_draw($data) {
    $chart = array(
        'id' => 'visualization',
        'type' => 'node_visualize',
        'data' => $data,
    );
    return d3_draw($chart);
}


function node_relation_create() {
    $sql = "SELECT entity_id AS originID, field_institutionen_target_id AS destinationID FROM {field_data_field_institutionen}";
    $nodes = db_query($sql);
    $nodes = $nodes->fetchAll();

    $data = array_merge(network_get_nodes($nodes), network_get_edges($nodes));

    dsm($data);

    return $data;
}


function network_get_nodes($nodes){
    $network_nodes = network_get_all_nodes($nodes);
    $nodes = array();
    foreach ($network_nodes as $node) {
        $network_node = array(
            'name' => $node,
        );
        array_push($nodes, $network_node);
    }

    $network_nodes = array(
        'nodes' => $nodes,
    );
    return $network_nodes;
}

function network_get_all_nodes($nodes) {
    $origin_ids = network_get_all_origin_ids($nodes);
    $destination_ids = network_get_all_destination_ids($nodes);
    $all_nodes = array_merge($origin_ids, $destination_ids);
    //dsm($all_nodes);
    $all_nodes = array_unique($all_nodes);
    return $all_nodes;
}

function network_get_all_origin_ids($nodes) {
    $origin_ids = array();
    foreach ($nodes as $node_data) {
        array_push($origin_ids, $node_data->originID);
    }
    return $origin_ids;
}

function network_get_all_destination_ids($nodes) {
$destinations_ids = array();
    foreach ($nodes as $node_data) {
        array_push($destinations_ids, $node_data->destinationID);
    }
    return $destinations_ids;
}

function network_get_edges($nodes) {
    $network_nodes = network_get_nodes($nodes);
    $network_edges = array();
    foreach ($nodes as $node) {
        //dsm("Kante zwischen " . $node->originID . " und " . $node->destinationID );
        $network_edge = array(
            //'source' => intval($node->originID),
            //'target' => intval($node->destinationID),
            'source' => intval(network_search_index($network_nodes['nodes'], $node->originID)),
            'target' => intval(network_search_index($network_nodes['nodes'], $node->destinationID)),
        );
        array_push($network_edges, $network_edge);
    }
    $network_edges = array(
        'edges' => $network_edges,
    );
    return $network_edges;
}

function network_search_index($network_nodes, $search_value) {
    for($i=0; $i<count($network_nodes);$i++) {
        //dsm($network_nodes[$i]['name']);
        if($network_nodes[$i]['name'] == $search_value) {
            break;
        }
    };
    return $i;
}

