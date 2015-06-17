/*
 * import.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "import.h"
#include <interpret_boolean/import.h>

hse::graph import_graph(const parse_boolean::guard &syntax, boolean::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	hse::graph result;
	hse::iterator b = result.create(hse::place());
	hse::iterator t = result.create(hse::transition(hse::transition::passive, import_cover(syntax, variables, default_id, tokens, auto_define)));
	hse::iterator e = result.create(hse::place());

	result.connect(b, t);
	result.connect(t, e);

	result.source.push_back(hse::state(vector<hse::reset_token>(1, hse::reset_token(b.index, false)), vector<hse::term_index>(), boolean::cover(1)));
	result.sink.push_back(hse::state(vector<hse::reset_token>(1, hse::reset_token(e.index, false)), vector<hse::term_index>(), boolean::cover(1)));
	return result;
}

hse::graph import_graph(const parse_boolean::assignment &syntax, boolean::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	hse::graph result;
	hse::iterator b = result.create(hse::place());
	hse::iterator t = result.create(hse::transition(hse::transition::active, import_cover(syntax, variables, default_id, tokens, auto_define)));
	hse::iterator e = result.create(hse::place());

	result.connect(b, t);
	result.connect(t, e);

	result.source.push_back(hse::state(vector<hse::reset_token>(1, hse::reset_token(b.index, false)), vector<hse::term_index>(), boolean::cover(1)));
	result.sink.push_back(hse::state(vector<hse::reset_token>(1, hse::reset_token(e.index, false)), vector<hse::term_index>(), boolean::cover(1)));
	return result;
}

hse::graph import_graph(const parse_hse::sequence &syntax, boolean::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.region != "")
		default_id = atoi(syntax.region.c_str());

	hse::graph result;

	for (int i = 0; i < (int)syntax.actions.size(); i++)
		if (syntax.actions[i] != NULL && syntax.actions[i]->valid)
		{
			if (syntax.actions[i]->is_a<parse_hse::parallel>())
				result.merge(hse::sequence, import_graph(*(parse_hse::parallel*)syntax.actions[i], variables, default_id, tokens, auto_define).wrap(), false);
			else if (syntax.actions[i]->is_a<parse_hse::condition>())
				result.merge(hse::sequence, import_graph(*(parse_hse::condition*)syntax.actions[i], variables, default_id, tokens, auto_define).wrap(), false);
			else if (syntax.actions[i]->is_a<parse_hse::loop>())
				result.merge(hse::sequence, import_graph(*(parse_hse::loop*)syntax.actions[i], variables, default_id, tokens, auto_define), false);
			else if (syntax.actions[i]->is_a<parse_boolean::assignment>())
				result.merge(hse::sequence, import_graph(*(parse_boolean::assignment*)syntax.actions[i], variables, default_id, tokens, auto_define), false);

			if (syntax.reset == 0 && i == 0)
				result.reset = result.source;
			else if (syntax.reset == i+1)
				result.reset = result.sink;
		}

	if (syntax.actions.size() == 0)
	{
		hse::iterator b = result.create(hse::place());
		hse::iterator t = result.create(hse::transition());
		hse::iterator e = result.create(hse::place());

		result.connect(b, t);
		result.connect(t, e);

		result.source.push_back(hse::state(vector<hse::reset_token>(1, hse::reset_token(b.index, false)), vector<hse::term_index>(), boolean::cover(1)));
		result.sink.push_back(hse::state(vector<hse::reset_token>(1, hse::reset_token(e.index, false)), vector<hse::term_index>(), boolean::cover(1)));

		if (syntax.reset >= 0)
			result.reset = result.source;
	}

	return result;
}

hse::graph import_graph(const parse_hse::parallel &syntax, boolean::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.region != "")
		default_id = atoi(syntax.region.c_str());

	hse::graph result;

	for (int i = 0; i < (int)syntax.branches.size(); i++)
		if (syntax.branches[i].valid)
			result.merge(hse::parallel, import_graph(syntax.branches[i], variables, default_id, tokens, auto_define), false);

	return result;
}

hse::graph import_graph(const parse_hse::condition &syntax, boolean::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.region != "")
		default_id = atoi(syntax.region.c_str());

	hse::graph result;

	for (int i = 0; i < (int)syntax.branches.size(); i++)
	{
		hse::graph branch;
		if (syntax.branches[i].first.valid)
			branch.merge(hse::sequence, import_graph(syntax.branches[i].first, variables, default_id, tokens, auto_define), false);
		if (syntax.branches[i].second.valid)
			branch.merge(hse::sequence, import_graph(syntax.branches[i].second, variables, default_id, tokens, auto_define), false);
		result.merge(hse::choice, branch, false);
	}

	return result;
}

hse::graph import_graph(const parse_hse::loop &syntax, boolean::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.region != "")
		default_id = atoi(syntax.region.c_str());

	hse::graph result;

	for (int i = 0; i < (int)syntax.branches.size(); i++)
	{
		hse::graph branch;
		if (syntax.branches[i].first.valid)
			branch.merge(hse::sequence, import_graph(syntax.branches[i].first, variables, default_id, tokens, auto_define), false);
		if (syntax.branches[i].second.valid)
			branch.merge(hse::sequence, import_graph(syntax.branches[i].second, variables, default_id, tokens, auto_define), false);
		result.merge(hse::choice, branch, false);
	}

	boolean::cover repeat = 0;
	for (int i = 0; i < (int)syntax.branches.size(); i++)
	{
		if (syntax.branches[i].first.valid)
			repeat |= import_cover(syntax.branches[i].first, variables, default_id, tokens, auto_define);
		else
			repeat = 1;
	}

	hse::iterator sm = result.create(hse::place());
	for (int i = 0; i < (int)result.source.size(); i++)
	{
		hse::iterator split_t = result.create(hse::transition(hse::transition::active, result.source[i].encodings));
		result.connect(sm, split_t);

		for (int j = 0; j < (int)result.source[i].tokens.size(); j++)
			result.connect(split_t, hse::iterator(hse::place::type, result.source[i].tokens[j].index));
	}

	for (int i = 0; i < (int)result.sink.size(); i++)
	{
		hse::iterator merge_t = result.create(hse::transition());
		result.connect(merge_t, sm);

		for (int j = 0; j < (int)result.sink[i].tokens.size(); j++)
			result.connect(hse::iterator(hse::place::type, result.sink[i].tokens[j].index), merge_t);
	}

	hse::iterator guard = result.create(hse::transition(hse::transition::passive, ~repeat));
	result.connect(sm, guard);
	hse::iterator arrow = result.create(hse::place());
	result.connect(guard, arrow);

	result.source.clear();
	result.source.push_back(hse::state(vector<hse::reset_token>(1, hse::reset_token(sm.index, false)), vector<hse::term_index>(), boolean::cover(1)));
	result.sink.clear();
	result.sink.push_back(hse::state(vector<hse::reset_token>(1, hse::reset_token(arrow.index, false)), vector<hse::term_index>(), boolean::cover(1)));

	if (result.reset.size() > 0)
	{
		result.source = result.reset;
		result.reset.clear();
	}

	return result;
}
