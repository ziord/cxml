/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXXPVISITORS_H
#define CXML_CXXPVISITORS_H

#include "cxxpast.h"

/*
 *  `path_spec` poison, for when `path_spec` of cxml_xp_step node, is 0, i.e.
 *  ->    name | .
 *  instead of:
 *  ->  /name | /. | //name | //.
 */
#define _CXML_XP_PS_POISON     (0)

/*************************************
 *        node freeing visitor       *
 *************************************
 */
void cxml_xp_fvisit(cxml_xp_astnode *ast_node);


/*************************************
 *          debug visitor            *
 *************************************
 */
void cxml_xp_dvisit(cxml_xp_astnode *ast_node);


/*************************************
 *  expression building visitor      *
 *************************************
 */
void cxml_xp_bvisit(cxml_xp_astnode *ast_node, cxml_string *acc);


/*************************************
 *  optimization checking visitor    *
 *************************************
 */
void cxml_xp_ovisit(cxml_xp_astnode *ast_node, _cxml_xp_ret_t *ret_type);


/*************************************
 *          eval node visitor        *
 *************************************
 */
void cxml_xp_visit(cxml_xp_astnode *);   // eval cxml_xp_visit

#endif //CXML_CXXPVISITORS_H
