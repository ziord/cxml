/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

char *wf_xml_6 = \
"<noodles>indomie<seasoning>maggi<br/>mr-chef</seasoning>super-pack<others></others></noodles>";

char *wf_xml_7 = \
"<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>";

char *wf_xml_9 = \
"<fruit><name>apple</name></fruit>";

char *wf_xml_09 = \
"<fruit a=\"boy\"><name>apple</name></fruit>";

char *wf_xml_10 = \
"<fruit><name>apple</name><name>banana</name></fruit>";

char *wf_xml_11 = \
"<fruit><!--some comment--><name>apple</name><!--another comment--></fruit>";

char *wf_xml_12 = \
"<x:fruit one=\"1\" two=\"2\" xmlns:x=\"uri\"></x:fruit>";

char *wf_xml_13 = \
"<a xmlns:x=\"uri\"><x:fruit one=\"1\" two=\"2\" x:three=\"3\" four=\"4\"></x:fruit></a>";

char *wf_xml_14 = \
"<fruit><?pi data?><class>basic<?pi data?></class><?pi data2?></fruit>";

char *wf_xml_15 = \
"<start><!--first comment--><begin><abc><!--maybe--></abc><xyz>foo<!--sometimes-->bar</xyz></begin><!--first comment--></start>";

char *wf_xml_16 = \
"<thesaurus>"
"begin"
"<entry>"
"benign"
"<term>successful</term>"
"<synonym>"
"misspelling"
"<term>successful</term>"
"<relationship>misspelling of</relationship>"
"stuff"
"</synonym>"
"</entry>"
"end"
"</thesaurus>";

char *wf_xml_dtd = \
"<!DOCTYPE people_list SYSTEM \"example.dtd\"><start>testing</start>";

char *wf_xml_xhdr = \
"<?xml version=\"1.0\"?><start>testing</start>";

char *wf_xml_plg = \
"<?xml version=\"1.0\"?><!DOCTYPE people_list SYSTEM \"example.dtd\"><start>testing</start>";

char *df_xml_1 = \
"12abcdefgh"
"<person fish=\"kote\">\n"
"        <gender>female</gender>\n"
"        <firstname>Anna</firstname>\n"
"        <lastname>Smith</lastname>\n"
"</person>";


static char *file = __FILE__;
static char *data_folder = NULL;

void set_data_path(){
    size_t  len = strlen(file) - 11;
    int data = 5;
    data_folder = malloc(len + data + 1);
    memcpy(data_folder, file, len);
    memcpy(data_folder + len, "data/", data);
    data_folder[len + data] = '\0';
}

void free_data_path(){
    free(data_folder);
}

char* get_file_path(char *file_name){
    cxml_string fp = new_cxml_string_s(data_folder);
    cxml_string_raw_append(&fp, file_name);
    return cxml_string_as_raw(&fp);
}

cxml_root_node *get_root(char *file_name, bool stream){
    char *fp = get_file_path(file_name);
    cxml_root_node *root = cxml_load_file(fp, stream);
    FREE(fp);
    return root;
}


/** fixtures **/

void fixture_no_fancy_printing_and_warnings(){
    cxml_reset_config();
    cxml_cfg_enable_fancy_printing(0);
    cxml_cfg_show_warnings(0);
}
