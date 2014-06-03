/**
* NOTICE: some changes have been made to maintain the integrity of hidden markov model:
* 1. state "E" emits "_" (underscore) with probability 1  
* 2. transition probability "E" -> "E" equals to 1
* 3. "E" has zero initial probability 
*
* SEE ALSO: documentation in html/index.html
*/
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

using namespace std;

void printResults(const map<string,double>&); 


int _tmain(int argc, _TCHAR* argv[]) {
	vector<string> stateSequence;
	vector<string> observationSequence;
	string   line;
	ifstream file("hmmdata.txt"); 
	if(!file) {
		cout << "File not found!"<< endl;
		return 0;
	} 
	getline(file, line); // first line
	while(getline(file,line)) {
		stringstream lineStream(line);
		string token;
		lineStream >> token; // number
		lineStream >> token;
		stateSequence.push_back(token);
		lineStream >> token;
		observationSequence.push_back(token);
	}

	stateSequence.push_back("E");  
	observationSequence.push_back("_");

	
	/* set the probabilities for HMM*/
	map<string,double> initialProbabilities; // all states must be here!
	initialProbabilities.insert(make_pair("St1",0.526));
	initialProbabilities.insert(make_pair("St2",0.474));
	initialProbabilities.insert(make_pair("E",0.0));
		
	typedef pair<string,string> Key;
	typedef map<Key, double> Map;

	Map transitionProbabilities;
	transitionProbabilities.insert(make_pair(Key ("St1","St1"), 0.969));
	transitionProbabilities.insert(make_pair(Key ("St1","St2"), 0.029));
	transitionProbabilities.insert(make_pair(Key ("St2","St1"), 0.063));
	transitionProbabilities.insert(make_pair(Key ("St2","St2"), 0.935));
	transitionProbabilities.insert(make_pair(Key ("St1","E"), 0.002));
	transitionProbabilities.insert(make_pair(Key ("St2","E"), 0.002));	
	transitionProbabilities.insert(make_pair(Key ("E","E"), 1));	

	Map emissionProbabilities;
	emissionProbabilities.insert(make_pair(Key ("St1","a"),0.005));
	emissionProbabilities.insert(make_pair(Key ("St1","b"),0.775));
	emissionProbabilities.insert(make_pair(Key ("St1","c"),0.220));
	emissionProbabilities.insert(make_pair(Key ("St2","a"),0.604));
	emissionProbabilities.insert(make_pair(Key ("St2","b"),0.277));
	emissionProbabilities.insert(make_pair(Key ("St2","c"),0.119));
	emissionProbabilities.insert(make_pair(Key ("E","_"),1.0)); // state E emits "_"  with probability 1

	/* construct HMM and perform calculations*/
	try {
		HiddenMarkovModel hmm(transitionProbabilities, emissionProbabilities, initialProbabilities);
		stateSequence.pop_back();

		cout << "--- Viterbi\n" << endl; 
		
		vector<string> predictedSequence =   hmm.calculateViterbiPath(observationSequence); 
		predictedSequence.pop_back(); // remove "E" state
		printResults(Evaluator::evaluate(stateSequence, predictedSequence, "St1"));

		cout << "\n--- Forward-Backward\n" << endl;

		predictedSequence = hmm.calculateSequenceByForwardBackward(observationSequence); 
		predictedSequence.pop_back(); // remove "E" state 
		printResults(Evaluator::evaluate(stateSequence, predictedSequence, "St1"));
	} catch (int exceptionNumber){
		switch(exceptionNumber) {
			case 0: cout << "Initial probabilities are wrong!\n" << endl; break;
			case 1: cout << "Transition probabilities are wrong!\n" << endl; break;
			case 2: cout << "Emission probabilities are wrong!\n" << endl; break;
			case 3: cout << "Lengths of actual and predicted sequences are different!\n" << endl; break;
			default: cout << "Something is wrong!\n" << endl; break;
		}
	}
	return 0;
}

void printResults(const map<string,double>& result) {
	for(map<string, double>::const_iterator iter = result.begin(); iter != result.end(); iter++) {
		cout << iter->first << " = " << iter->second << endl;
	}
}
