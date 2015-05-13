/*
 * export.h
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include <common/standard.h>

#include <boolean/variable.h>
#include <boolean/cube.h>
#include <boolean/cover.h>

#include <parse_boolean/variable_name.h>
#include <parse_boolean/internal_choice.h>
#include <parse_boolean/disjunction.h>

#include <parse_hse/sequence.h>
#include <parse_hse/parallel.h>
#include <parse_hse/condition.h>
#include <parse_hse/loop.h>

#include <hse/graph.h>

#include <interpret_boolean/export.h>

#ifndef interpret_hse_export_h
#define interpret_hse_export_h

parse_hse::sequence export_sequence(hse::iterator &branch, const hse::graph &g, boolean::variable_set &v);
parse_hse::condition export_condition(vector<hse::iterator> &branches, const hse::graph &g, boolean::variable_set &v);
parse_hse::parallel export_parallel(vector<hse::iterator> &branches, const hse::graph &g, boolean::variable_set &v);

#endif
