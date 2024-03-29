//  C_Groups
//  $Id$
//  
//  Copyright (2008) Sandia Corporation. Under the terms of
//  Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
//  Government retains certain rights in this software.
//  
//  Hermes is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//  
//  Hermes is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//  
//  You should have received a copy of the GNU Lesser General
//  Public License along with Hermes.  If not, see
//  <http://www.gnu.org/licenses/>.
//  

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>

#ifdef WIN32sys
# include "winrtl.h"
#else
# include <unistd.h>
#endif

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::ostream;

#ifdef BOOTSTRAP
// dummy usage function since txt2cwriter needed to make txt2cwriter_usage.h
void txt2cwriter_usage(std::ostream &o, int mode) { }
#else
# include "txt2cwriter_usage.h"
#endif

static void usage(const string &cmd, int status)
{
  ostream *o = &cout;
  if ( status ) o = &cerr;

  *o << "Usage: " << cmd << " [-h] [-s] [-c comment_char] [-p prefix_string]\n"
        "       [-w max_width] text_file c_file " << endl;

  exit(status);
}

static int read_comparator(char *endptr, const string &compsymbs, int dflt)
{
  int ct = dflt;
  string::size_type idx;
  if ( *endptr ) {
    if((idx=compsymbs.find(*endptr)) != string::npos) {
      ct = idx;
      // if '<' or '>', look for '<=' or '>='
      if ( ct<2 && *(++endptr) && *endptr == '=' ) ct = 5 - ct;
    }
  }
  return ct;
}

int main(int argc, char *argv[])
{
  const string cxx_exts = ":C:cc:cp:cpp::cxx:CPP:c++:";
  const string f_exts = ":F:f:F90:f90:";
  const string compsymbs = "<>=!";
  const string whtspc = " \t";
  string comps[6];
  char cmmnt = '!';  // '!' is default comment character
  int  width = 78;   // 78 is default line width for output code
  int error = 0;
  bool cpp_ok = true;
  bool short_mode = false;
  string pad = "";

  enum output_modes{ C_MODE, CXX_MODE, F_MODE };

  // get command "basename"
  string cmd = argv[0];
  string::size_type idx = cmd.rfind('/');
  if ( idx != string::npos ) cmd.erase(0,idx+1);
  argv[0] = (char *) cmd.c_str(); // use base name for getopt messages

  int c = -1;
  for(int i=1;i<argc;++i) {
    if (strcmp(argv[i],"-h")==0) { c = 1; break; }
    if (strcmp(argv[i],"--help")==0) { c = 0; break; }
  }
  if (c >= 0) {
    txt2cwriter_usage(cout,c);
    return 0;
  }
  // check for valid options
  while ( (c=getopt(argc, argv, "hsc:p:w:")) != -1 ) {
    switch (c) {
    case 's':
      short_mode = true;
      break;
    case 'c':
      cmmnt = optarg[0];
      break;
    case 'p':
      pad = optarg;
      break;
    case 'w':
      { int val = std::atoi(optarg);
        if (val) width = val;
        else cerr <<"Can't parse -w argument -- width set to " <<width <<endl;
      }
      break;
 
    case '?':  // invalid option
      error = 1;
      break;

    default:  // getopt should never return something that goes here !! 
      error = 1;
      cerr <<"?? getopt returned unexpected character code: " <<c <<endl;
    }    
  }

  // shift away the option arguments
  argv += optind;
  argc -= optind;

  if ( error || argc != 2 ) {
    if (argc != 2) cerr << cmd << ": Two arguments required" << endl;
    txt2cwriter_usage(cout,2);
    exit(1);
  }

  string txtfile = argv[0];  // input text file
  string cfile = argv[1];    // output C or C++ source file

  // open input file
  std::ifstream inp(argv[0],std::ios::in);
  if ( ! inp ) {
    cerr << cmd << ": cannot open input file \"" << txtfile << "\"" << endl;
    exit(1);
  }

  // Scan beginning comment lines, looking for EXTENSION directive of the form
  //    "! EXTENSION: C" (assuming default comment character)
  // The third token (C in the example above) will be used to determine the
  // language output mode, rather of using the extension from the output
  // file's  name.
  string ext = "";
  string tstr;
  while ( getline(inp,tstr) ) {
    string s;
    if ( tstr[0] == cmmnt ) {
      std::istringstream t(tstr);
      t >> s;
      if (s.size()>1 || t.eof()) continue;
      t >> s;
      if ( s != "EXTENSION:" || t.eof()) continue;
      t >> s;
      ext = ":" + s + ":";
      break;
    }
    else break;
  }
  inp.seekg(0);

  idx = cfile.rfind('.');
  string funct_name = cfile;
  if ( idx != string::npos ) {
    // Look for extension of output file name, and see if it matches a C++ ext.
    if ( ext.empty() ) ext = ":" + cfile.substr(idx+1) + ":";
    // take function name from file prefix
    funct_name.erase(idx);
  }
  // If we have a extension value, use it to determine the language output mode
  output_modes out_mode = C_MODE;  // C output mode is default
  if ( !ext.empty() ) {
    if ( cxx_exts.find(ext) != string::npos ) out_mode = CXX_MODE;
    else if ( f_exts.find(ext) != string::npos ) {
      out_mode = F_MODE;
      width = std::min(72,width);
      if ( ext.at(1) != 'F' ) cpp_ok = false;
    }
    // else -- C_MODE is default
  }
  // remove directory name for function name, if it has one
  idx = funct_name.rfind('/');
  if ( idx != string::npos ) funct_name.erase(0,idx+1);
  // if funct_name still has a dot in it, its an error !!
  idx = funct_name.find('.');
  if ( idx != string::npos ) {
    cerr << cmd << ": Output file name \"" << cfile
         << "\" cannot contain more than one '.' character in its basename"
         << endl;
    exit(1);
  }

  // open output file
  std::ofstream out(argv[1]);
  if ( ! out ) {
    cerr << cmd << ": cannot open output file \"" << cfile << "\"" << endl;
    exit(1);
  }

  // write file header for C or C++ code
  string testarg; // tester's calling argument for standard output
  if ( out_mode == C_MODE ) {
    out << "#include <stdio.h>" << endl;
    if ( short_mode ) out << "#include <stdlib.h>" << endl;
    out << "\nvoid " << funct_name << "(FILE *o";
    if ( short_mode ) out << ", int mode";
    out << ")\n{" << endl;
    testarg = "stdout";
  }
  else if (out_mode == F_MODE ) {
    out << "      subroutine " << funct_name << "(lun";
    if ( short_mode ) out << ", mode)\n      integer mode" << endl;
    else out << ")" << endl;
    out << "      integer lun" << endl;
  }
  else {
    out << "#include <iostream>" << endl;
    out << "#include <string>" << endl;
    if ( short_mode ) out << "#include <cstdlib>" << endl;
    out << "\nvoid " << funct_name << "(std::ostream &o";
    if ( short_mode ) out << ", int mode";
    out << ")\n{" << endl;
    testarg = "std::cout";
   }
  
  const int bufsize = 1024;
  char line[bufsize];
  string prefix, suffix;
  // set prefix and suffix for write command for C or C++
  string suf_cont = "\"";
  string short_if = "  if ( mode ";
  string short_suf = " ) return;";
  string start_if = "  if ( mode ";
  string start_suf = " ) {";
  string end_if = "  }";
  comps[0] = "< ";
  comps[1] = "> ";
  comps[2] = "== ";
  comps[3] = "!= ";
  comps[4] = ">= ";
  comps[5] = "<= ";
  if ( out_mode == C_MODE ) {
    prefix = "  fprintf(o,\"";
    suffix = "\\n\");";
  }
  else if (out_mode == F_MODE ) {
    prefix = "      write(lun,1) '";
    suffix = "'";
    suf_cont = "'";
    short_if = "      if ( mode ";
    short_suf = " ) return";
    start_if = "      if ( mode ";
    start_suf = " ) then";
    end_if = "      endif";
    comps[0] = ".LT. ";
    comps[1] = ".GT. ";
    comps[2] = ".EQ. ";
    comps[3] = ".NE. ";
    comps[4] = ".GE. ";
    comps[5] = ".LE. ";
  }
  else {
    prefix = "  o << \"";
    suffix = "\\n\";";
  }
  int npre = prefix.length();
  int max1line = width - npre - suffix.length();
  string blank(npre,' '); // prefix for wrapped lines

  string fempty = "";
  if ( out_mode == F_MODE ) {
    blank.at(5) = '&';
    blank.replace(npre-4,4,"// '");
    fempty = prefix.substr(0,npre-2);
  }
  else blank.at(npre-1) = '"';

  int npad = pad.length();
  char *buf = line + npad;
  if ( npad ) strcpy(line,pad.c_str());
  int buflen = bufsize - npad;

  int nest = 0;
  char *endptr = 0;
  while ( inp.getline(buf,buflen) ) {
    if ( buf[0] == cmmnt ) { // skip lines w/ comment char in col 1
      if ( short_mode && buf[1] == '#' ) {
        if ( strncmp(buf+2,"start",5) == 0 ) {
          int mode = std::strtol(buf+7,&endptr,10);
          while ( whtspc.find(*endptr) != string::npos ) endptr++;
          int ct = read_comparator(endptr, compsymbs, 0);
          string comp = comps[ct];
          out << start_if << comp << mode << start_suf << endl;
          nest++;
        }
        else if ( strncmp(buf+2,"stop",4) == 0 ) {
          out << end_if << endl;
          nest--;
          if ( nest < 0 ) break; // a "stop" requires previous "start"
        }
        else {
          int mode = std::strtol(buf+2,&endptr,10);
          while ( whtspc.find(*endptr) != string::npos ) endptr++;
          int ct = read_comparator(endptr, compsymbs, 4);
          string comp = comps[ct];
          out << short_if << comp << mode << short_suf << endl;
        }
      }
      continue;  
    }
    int top = strlen(line);
    if ( top == 0 ) {  // special handling for empty lines
      if ( out_mode != F_MODE ) out << prefix << suffix << endl;
      else out << fempty << endl;
    }
    else {
      string sline(line);
      string::size_type loc = 0;
      // need to escape special characters
      if ( out_mode != F_MODE ) { // '\' and '"' for C and C++
        while ( (idx = sline.find_first_of("\\\"",loc)) != string::npos ) {
          sline.insert(idx,1,'\\');
          loc = idx + 2; ++top;
        }
      }
      else { // "'" for Fortran
        while ( (idx = sline.find_first_of("'",loc)) != string::npos ) {
          sline.insert(idx,1,'\'');
          loc = idx + 2; ++top;
        }
      }
      string *start = &prefix;
      string *finish = &suf_cont;
      int offset = 0;
      while ( offset < top ) {
        int nextlen = std::min(max1line,top - offset);
        string part = sline.substr(offset,nextlen);
        // DON'T want to separate a backslash from a trailing special character
        if ( part.at(nextlen-1) == '\\' &&  part.at(nextlen-2) != '\\' ) {
          part.erase(--nextlen);
        }
        if ( top-offset <= max1line ) finish = &suffix;
        out << *start << part << *finish << endl;
        offset += nextlen;
        start = &blank;
      }
    }
  }
  if ( nest != 0 ) {
    cerr << cmd << ": improper nesting of " << cmmnt << "#start/stop directives"
         << endl;
    exit(nest);
  }
    
  // write function ending for Fortran or C/C++ code
  if ( out_mode == F_MODE ) out << "    1 format(a)\n"
                                << "      return\n" 
                                << "      end" << endl;
  else out << "}" << endl;

  // write test program for Fortran or C/C++ code, if okay
  if ( cpp_ok ) {
    out << "#ifdef TEST" << endl;
    if ( out_mode != F_MODE ) {
      out << "int main(int argc, char *argv[])\n{" << endl;
      if ( short_mode ) {
        out << "  int mode = 0;" << endl;
        if ( out_mode == C_MODE )
          out << "  if ( argc > 1 ) mode = atoi(argv[1]);" << endl;
        else
          out << "  if ( argc > 1 ) mode = std::atoi(argv[1]);" << endl;
      }
      out << "  " << funct_name << "(" << testarg;
      if ( short_mode ) out << ", mode";
      out << ");\n  return 0;\n}" << endl;
    }
    else {
      out << "      program test\n"
          << "      call " << funct_name << "(6";
      if ( short_mode ) out << ",0)\n"
                            << "      write(6,*) '----------------------'\n"
                            << "      call " << funct_name << "(6,TEST";
      out << ")\n"
          << "      end" << endl;
    }
    out << "#endif" << endl;
  }
 
  return 0;
}
