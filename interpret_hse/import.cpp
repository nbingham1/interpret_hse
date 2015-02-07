/*
 * import.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "import.h"
#include <interpret_boolean/import.h>

vector<hse::iterator> import_graph(const parse_boolean::guard &syntax, boolean::variable_set &variables, hse::graph &g, vector<hse::iterator> m, bool auto_define)
{
	if (syntax.valid)
		return g.push_back(m, hse::transition(hse::transition::passive, import_cover(syntax, variables, auto_define)), 1);
	else
		return g.push_back(m, hse::transition(), 1);
}

vector<hse::iterator> import_graph(const parse_boolean::assignment &syntax, boolean::variable_set &variables, hse::graph &g, vector<hse::iterator> m, bool auto_define)
{
	if (syntax.valid)
		return g.push_back(m, hse::transition(hse::transition::active, import_cover(syntax, variables, auto_define)), 1);
	else
		return g.push_back(m, hse::transition(), 1);
}

vector<hse::iterator> import_graph(const parse_hse::sequence &syntax, boolean::variable_set &variables, hse::graph &g, vector<hse::iterator> m, bool auto_define)
{
	if (syntax.valid)
	{
		bool first = true;
		for (int i = 0; i < (int)syntax.actions.size(); i++)
		{
			if (syntax.actions[i]->valid)
			{
				if (!first)
					m = g.push_back(m, hse::place(), 1);
				else
					first = false;

				if (syntax.actions[i]->is_a<parse_hse::parallel>())
					m = import_graph(*(parse_hse::parallel*)syntax.actions[i], variables, g, m, auto_define);
				else if (syntax.actions[i]->is_a<parse_hse::sequence>())
					m = import_graph(*(parse_hse::sequence*)syntax.actions[i], variables, g, m, auto_define);
				else if (syntax.actions[i]->is_a<parse_hse::condition>())
					m = import_graph(*(parse_hse::condition*)syntax.actions[i], variables, g, m, auto_define);
				else if (syntax.actions[i]->is_a<parse_hse::loop>())
					m = import_graph(*(parse_hse::loop*)syntax.actions[i], variables, g, m, auto_define);
				else if (syntax.actions[i]->is_a<parse_boolean::assignment>())
					m = import_graph(*(parse_boolean::assignment*)syntax.actions[i], variables, g, m, auto_define);
			}
		}

		return m;
	}
	else
		return g.push_back(m, hse::transition(), 1);
}

vector<hse::iterator> import_graph(const parse_hse::parallel &syntax, boolean::variable_set &variables, hse::graph &g, vector<hse::iterator> m, bool auto_define)
{
	if (syntax.valid)
	{
		m = g.push_back(m, hse::transition(), 1);

		vector<hse::iterator> ends;

		for (int i = 0; i < (int)syntax.branches.size(); i++)
		{
			if (syntax.branches[i]->valid)
			{
				vector<hse::iterator> temp;
				temp = g.push_back(ends, hse::place(), 1);

				if (syntax.branches[i]->is_a<parse_hse::parallel>())
					temp = import_graph(*(parse_hse::parallel*)syntax.branches[i], variables, g, temp, auto_define);
				else if (syntax.branches[i]->is_a<parse_hse::sequence>())
					temp = import_graph(*(parse_hse::sequence*)syntax.branches[i], variables, g, temp, auto_define);
				else if (syntax.branches[i]->is_a<parse_hse::condition>())
					temp = import_graph(*(parse_hse::condition*)syntax.branches[i], variables, g, temp, auto_define);
				else if (syntax.branches[i]->is_a<parse_hse::loop>())
					temp = import_graph(*(parse_hse::loop*)syntax.branches[i], variables, g, temp, auto_define);
				else if (syntax.branches[i]->is_a<parse_boolean::assignment>())
					temp = import_graph(*(parse_boolean::assignment*)syntax.branches[i], variables, g, temp, auto_define);

				temp = g.push_back(ends, hse::place(), 1);
				ends.insert(ends.end(), temp.begin(), temp.end());
			}
		}

		return g.push_back(ends, hse::transition(), 1);
	}
	else
		return g.push_back(m, hse::transition(), 1);
}

vector<hse::iterator> import_graph(const parse_hse::condition &syntax, boolean::variable_set &variables, hse::graph &g, vector<hse::iterator> m, bool auto_define)
{
	if (syntax.valid)
	{
		vector<hse::iterator> result;
		for (int i = 0; i < (int)syntax.branches.size(); i++)
		{
			vector<hse::iterator> temp = m;

			temp = import_graph(syntax.branches[i].first, variables, g, temp, auto_define);

			if (syntax.branches[i].first.valid && syntax.branches[i].second.valid)
				temp = g.push_back(temp, hse::place(), 1);

			temp = import_graph(syntax.branches[i].second, variables, g, temp, auto_define);

			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}
	else
		return g.push_back(m, hse::transition(), 1);
}

vector<hse::iterator> import_graph(const parse_hse::loop &syntax, boolean::variable_set &variables, hse::graph &g, vector<hse::iterator> m, bool auto_define)
{
	if (syntax.valid)
	{
		vector<hse::iterator> result;
		for (int i = 0; i < (int)syntax.branches.size(); i++)
		{
			vector<hse::iterator> temp = m;

			temp = import_graph(syntax.branches[i].first, variables, g, temp, auto_define);

			if (syntax.branches[i].first.valid && syntax.branches[i].second.valid)
				temp = g.push_back(temp, hse::place(), 1);

			temp = import_graph(syntax.branches[i].second, variables, g, temp, auto_define);

			result.insert(result.end(), temp.begin(), temp.end());
		}

		g.connect(result, m);

		boolean::cover exit = 1;
		for (int i = 0; i < (int)syntax.branches.size(); i++)
			exit &= import_cover(syntax.branches[i].first, variables, auto_define);

		return g.push_back(m, hse::transition(hse::transition::passive, exit), 1);
	}
	else
		return g.push_back(m, hse::transition(), 1);
}
