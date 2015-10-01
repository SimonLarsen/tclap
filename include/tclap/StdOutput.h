// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/****************************************************************************** 
 * 
 *  file:  StdOutput.h
 * 
 *  Copyright (c) 2004, Michael E. Smoot
 *  All rights reverved.
 * 
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *  
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 *  DEALINGS IN THE SOFTWARE.  
 *  
 *****************************************************************************/ 

#ifndef TCLAP_STDCMDLINEOUTPUT_H
#define TCLAP_STDCMDLINEOUTPUT_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <algorithm>

#include <tclap/CmdLineInterface.h>
#include <tclap/CmdLineOutput.h>
#include <tclap/XorHandler.h>
#include <tclap/Arg.h>

namespace TCLAP {
	class StdOutput : public CmdLineOutput {
		public:
			void usage(CmdLineInterface &c);
			virtual void version(CmdLineInterface& c);
			virtual void failure(CmdLineInterface& c, ArgException& e);

		protected:
        	void shortUsage(CmdLineInterface& c, std::ostream& os) const;
			void longUsage(CmdLineInterface& c, std::ostream& os) const;
			void spacePrint(std::ostream& os, const std::string& s, int maxWidth, int indentSpaces, int secondLineOffset) const;
	};

	inline void StdOutput::usage(CmdLineInterface& _cmd) {
		std::string message = _cmd.getMessage();
		spacePrint(std::cout, message, 80, 0, 0);

		std::cout << std::endl << "Usage: ";
		shortUsage(_cmd, std::cout);
		std::cout << std::endl << "Options: " << std::endl;
		longUsage(_cmd, std::cout);
	}

	inline void StdOutput::version(CmdLineInterface& _cmd) {
		std::cout << _cmd.getProgramName() << " " << _cmd.getVersion() << std::endl << std::endl;
		std::cout << "Written by " << _cmd.getAuthor() << std::endl;
	}

	inline void StdOutput::failure(CmdLineInterface& _cmd, ArgException& e) {
		std::cerr << "error: " << e.argId() << std::endl
				  << "       " << e.error() << std::endl << std::endl;

		if (_cmd.hasHelpAndVersion()) {
			std::cerr << "Usage: ";
			shortUsage(_cmd, std::cerr);	
			std::cerr << std::endl << "For complete USAGE and HELP type: " 
					  << std::endl << "   " << _cmd.getProgramPath() << " --help" 
					  << std::endl << std::endl;
		} else {
			usage(_cmd);
		}

		throw ExitException(1);
	}

	inline void StdOutput::shortUsage(CmdLineInterface& _cmd, std::ostream& os) const {
		std::list<Arg*> argList = _cmd.getArgList();
		XorHandler xorHandler = _cmd.getXorHandler();
		std::vector<std::vector<Arg*> > xorList = xorHandler.getXorList();

		std::string s = _cmd.getProgramPath() + " [OPTIONS]";

		// first the xor
		for(int i = 0; static_cast<unsigned int>(i) < xorList.size(); i++) {
			s += " {";
			for (ArgVectorIterator it = xorList[i].begin(); it != xorList[i].end(); it++) {
				s += (*it)->shortID() + "|";
			}

			s[s.length()-1] = '}';
		}

		// then the rest
		for(ArgListIterator it = argList.begin(); it != argList.end(); it++) {
			if (!xorHandler.contains((*it)) && !(*it)->isIgnoreable()) {
				s += " " + (*it)->shortID();
			}
		}

		spacePrint(os, s, 80, 0, 5);
	}

	inline void StdOutput::longUsage(CmdLineInterface& _cmd, std::ostream& os) const {
		std::list<Arg*> argList = _cmd.getArgList();
		XorHandler xorHandler = _cmd.getXorHandler();
		std::vector< std::vector<Arg*> > xorList = xorHandler.getXorList();

		// first the xor 
		for(int i = 0; static_cast<unsigned int>(i) < xorList.size(); i++) {
			for(ArgVectorIterator it = xorList[i].begin(); it != xorList[i].end(); it++) {
				std::string s = (*it)->longID();
				int spacing = std::max(35 - (int)s.length(), 2);

				s += std::string(spacing, ' ');
				s += (*it)->getDescription();
				
				spacePrint(os, s, 80, 2, 35);

				os << std::endl;
				if(it+1 != xorList[i].end()) {
					spacePrint(os, "-- OR --", 80, 9, 0);
				}
				os << std::endl;
			}
		}

		// then the rest
		for(ArgListIterator it = argList.begin(); it != argList.end(); it++) {
			if(!xorHandler.contains((*it))
			&& (*it)->getFlag() != "") {
				std::string s = (*it)->longID();
				int spacing = std::max(35 - (int)s.length(), 2);

				s += std::string(spacing, ' ');
				s += (*it)->getDescription();

				spacePrint(os, s, 80, 2, 35);

				os << std::endl;
			}
		}
	}

	inline void StdOutput::spacePrint(std::ostream& os, const std::string& s, int maxWidth, int indentSpaces, int secondLineOffset) const {
		int len = static_cast<int>(s.length());

		if((len + indentSpaces > maxWidth) && maxWidth > 0) {
			int allowedLen = maxWidth - indentSpaces;
			int start = 0;
			while(start < len) {
				// find the substring length
				// int stringLen = std::min<int>(len - start, allowedLen);
				// doing it this way to support a VisualC++ 2005 bug 
				using namespace std; 
				int stringLen = min<int>(len - start, allowedLen);

				// trim the length so it doesn't end in middle of a word
				if(stringLen == allowedLen) {
					while(stringLen >= 0 && s[stringLen+start] != ' ' && s[stringLen+start] != ',' && s[stringLen+start] != '|') stringLen--;
				}

				// ok, the word is longer than the line, so just split 
				// wherever the line ends
				if(stringLen <= 0) stringLen = allowedLen;

				// check for newlines
				for(int i = 0; i < stringLen; i++) {
					if(s[start+i] == '\n') {
						stringLen = i+1;
					}
				}

				// print the indent	
				for(int i = 0; i < indentSpaces; i++) os << " ";

				if(start == 0) {
					// handle second line offsets
					indentSpaces += secondLineOffset;

					// adjust allowed len
					allowedLen -= secondLineOffset;
				}

				os << s.substr(start,stringLen) << std::endl;

				// so we don't start a line with a space
				while(s[stringLen+start] == ' ' && start < len) {
					start++;
				}
		
				start += stringLen;
			}
		} else {
			for(int i = 0; i < indentSpaces; i++) os << " ";
			os << s << std::endl;
		}
	}
} //namespace TCLAP

#endif 
