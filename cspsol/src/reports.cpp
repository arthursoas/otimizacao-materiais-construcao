/* File: reports.cpp
 * Description: Functions related to creating solution reports.
 */

#include<iostream>
#include<iomanip>
#include<ostream>
#include<string>
#include<cmath>
#include<cstring>

#include "pattern.h"
#include "cmdline.h"
#include "extern.h"

using namespace std;

/*-------------------------------------------------------------------
 * Precondition: Call to function store_solution.
 * Print stored solution patterns completely. 
-------------------------------------------------------------------*/
void BBNode::print_solution(glp_prob * master_lp, OrderWidthContainer& ow_set)
{

	PatternIterator pat_iter = pattern_list.begin();	
	for(; pat_iter != pattern_list.end(); pat_iter++) {
		double x = (*pat_iter)->get_int_sol();
		if(fabs(x) <= EPSILON)
			continue;

		// Store order width solution, to be used later.
		for(int i = 1; i <= (*pat_iter)->nzcnt; i++) {
			int ow_row_index = (*pat_iter)->ind[i];
			int ow_count = (*pat_iter)->val[i];

			OrderWidth * ow;
		  ow = OrderWidth::find_orderwidth(ow_set, ow_row_index);
			// Append
			int sol = ow->get_solution() + ow_count * ceil(x);
			ow->set_solution(sol);
		}
	}

	for(int i = 0; i < (int)option->rformats.size(); i++)
	{
		char * filename = option->rfilenames[i];

		/* Restore cout to original state. */
		option->restore_cout();

		if(option->rformats[i] == TEXT) {

			if(!strcmp(filename, "stdout"))
				print_text_report(cout, master_lp, ow_set);
			else {
				ofstream fout(filename);
				cout<<"Generating TEXT solution report "<<filename<<endl;
				print_text_report(fout, master_lp, ow_set);
			}

		} else if(option->rformats[i] == HTML) {

			if(!strcmp(filename, "stdout"))
				print_json_report(cout, master_lp, ow_set);
			else {
				ofstream fout(filename);
				cout<<"HTML report: Data file '"<<filename<<"' created."<<endl; 
				cout<<"+ Keep it in same folder as 'data/index.html'."<<endl; 
				cout<<"+ Open 'index.html' in browser to view HTML report."<<endl<<endl;
				print_json_report(fout, master_lp, ow_set);
			}
		} else if(option->rformats[i] == XML) {

			if(!strcmp(filename, "stdout"))
				print_xml_report(cout, master_lp, ow_set);
			else {
				ofstream fout(filename);
				cout<<"Generating solution report "<<filename<<endl;
				print_xml_report(fout, master_lp, ow_set);
			}
		}
	}
}

/*-------------------------------------------------------------------
 * Print solution report to 'fout' in text format. 
-------------------------------------------------------------------*/
void BBNode::print_text_report(ostream& fout, glp_prob * master_lp, 
								OrderWidthContainer& ow_set)
{
	double x;
	fout << endl << " # Solution Report # "<< endl << endl;
	fout << "Best integer obj. func. value = " << BBNode::get_best_int_obj_val() << endl;

	PatternIterator pat_iter = pattern_list.begin();	
	for(; pat_iter != pattern_list.end(); pat_iter++) {
		
		x = (*pat_iter)->get_int_sol();
		if(fabs(x) <= EPSILON)
			continue;

		fout << "Pattern count = " << setw(4) << x <<": ";

		for(int i = 1; i <= (*pat_iter)->nzcnt; i++) {
			int ow_row_index = (*pat_iter)->ind[i];
			double ow_count = (*pat_iter)->val[i];

			OrderWidth * ow;
		  ow = OrderWidth::find_orderwidth(ow_set, ow_row_index);
			fout << setw(5) << ow->get_width() << " x " << setw(2) << ow_count <<", ";
		}
		fout << endl;		
	}
	fout << endl;
}

/*-------------------------------------------------------------------
 * Print solution report to 'fout' in XML format. 
-------------------------------------------------------------------*/
void BBNode::print_xml_report(ostream& fout, glp_prob * master_lp, 
								OrderWidthContainer& ow_set) 
{
	double x;
  int patcnt;
  int cutcnt;

  fout << "<solution>"<< endl;
  fout << "\t<stockcount>" << BBNode::get_best_int_obj_val() << "</stockcount>" << endl;

	PatternIterator pat_iter = pattern_list.begin();	
	for(; pat_iter != pattern_list.end(); pat_iter++) {
                
  	//col_index = (*pat_iter)->get_master_col_num();
    //x = glp_get_col_prim(master_lp, col_index);
    x = (*pat_iter)->get_int_sol();
    if(abs(x) <= EPSILON)
    	continue;
    for (patcnt = 0; patcnt < lround(x); patcnt++) {
    	fout<< "\t<pattern>"<<endl;

      for(int i = 1; i <= (*pat_iter)->nzcnt; i++) {
      	int ow_row_index = (*pat_iter)->ind[i];
        double ow_count = (*pat_iter)->val[i];

        OrderWidth * ow;
        ow = OrderWidth::find_orderwidth(ow_set, ow_row_index);
        for (cutcnt=0;cutcnt<ow_count;cutcnt++)
        {
        	fout<<"\t\t<cut>"<<ow->get_width() << "</cut>" << endl;
        }
      }
      fout << "\t</pattern>" << endl;
    }
  }
  fout << "</solution>"<< endl;
}

/*-------------------------------------------------------------------
 * Print solution report to 'fout' in XML format. 
-------------------------------------------------------------------*/
void BBNode::print_json_report(ostream& fout, glp_prob * master_lp, 
								OrderWidthContainer& ow_set) 
{
	double x;
  fout << "{"<< endl;
  fout << "\t\"run\": {"<< endl;
  fout << "\t\t\"Optimal\":" << BBNode::get_best_int_obj_val() << "," << endl;
  fout << "\t\t\"Solution\":["<<endl;

	PatternIterator pat_iter = pattern_list.begin();
	for(; pat_iter != pattern_list.end(); pat_iter++) {
		
		x = (*pat_iter)->get_int_sol();
		if(fabs(x) <= EPSILON)
			continue;

    if(pat_iter != pattern_list.begin())
			fout << "," << endl;

		fout << "\t\t\t{\"pattern\":[";
		for(int i = 1; i <= (*pat_iter)->nzcnt; i++) {
			int ow_row_index = (*pat_iter)->ind[i];
			double ow_count = (*pat_iter)->val[i]; // TODO: ceil ?

			OrderWidth * ow;
		  ow = OrderWidth::find_orderwidth(ow_set, ow_row_index);
			if(i != 1)
				fout << ", ";

			for(int c = 0; c < ow_count; c++)
			{
				if(c != 0)
		 			fout << ", ";
		 		fout << ow->get_width();
			}
		}
		fout << "], \"count\":" << x << "}";
	}

  fout << endl << "\t\t],"<< endl; // SolutionPattern

  fout << "\t\t\"SolutionWidth\":["<<endl;
	OrderWidthIterator ow_iter = ow_set.begin();	
	for(; ow_iter != ow_set.end(); ow_iter++) {
		double width = (*ow_iter)->get_width();
		double demand = (*ow_iter)->get_demand();
		double solution = (*ow_iter)->get_solution();

		if(ow_iter != ow_set.begin())
			fout << "," << endl;
		fout << "\t\t\t{\"width\":" << width << ", ";
		fout << "\"demand\":" << demand << ", ";
		fout << "\"solution\":" << solution << "}";
	}

  fout << "\t\t]"<< endl; // SolutionWidth

  fout << "\t}"<< endl;   // Run
  fout << "}"<< endl;
}
