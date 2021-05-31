/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXMLTESTS_CXFIXTURE_H
#define CXMLTESTS_CXFIXTURE_H
#include "cxtest.h"
#include <cxml/cxml.h>

#define deb()

struct Data{
    int data;
};

extern char *wf_xml_6;
extern char *wf_xml_7;
extern char *wf_xml_9;
extern char *wf_xml_09;
extern char *wf_xml_10;
extern char *wf_xml_11;
extern char *wf_xml_12;
extern char *wf_xml_13;
extern char *wf_xml_14;
extern char *wf_xml_15;
extern char *wf_xml_16;
extern char *wf_xml_dtd;
extern char *wf_xml_xhdr;
extern char *wf_xml_plg;
extern char *df_xml_1;

void set_data_path();
void free_data_path();
char* get_file_path(char *file_name);
cxml_root_node *get_root(char *file_name, bool stream);
void fixture_no_fancy_printing_and_warnings();

#endif //CXMLTESTS_CXFIXTURE_H
