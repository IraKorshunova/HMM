#include "StdAfx.h"
#include <iostream>
using namespace std;

map<string,double> Evaluator::evaluate(const vector<string>& predictedSequence,
									   const vector<string>& actualSequence, 
									   const string& classLabel) {

		if(predictedSequence.size() != actualSequence.size()) throw 3; 

		// initialization
		int truePositives = 0;
		int trueNegatives = 0;
		int falsePositives = 0;
		int falseNegatives = 0;

		// calculate confusion matrix
		for(vector<int>::size_type i = 0; i < actualSequence.size(); i++) {
			if(actualSequence[i] == classLabel && predictedSequence[i] == classLabel) truePositives++;
			if(actualSequence[i] != classLabel && predictedSequence[i] != classLabel) trueNegatives++;
			if(actualSequence[i] == classLabel && predictedSequence[i] != classLabel) falseNegatives++;
			if(actualSequence[i] != classLabel && predictedSequence[i] == classLabel) falsePositives++;
		}
		// calculate F score
		double precision =(double) truePositives / (truePositives + falsePositives); 
		double recall = (double)truePositives / (truePositives + falseNegatives);
		double fScore = 2 * precision*recall / (precision + recall);

		// mapping
		map<string,double> nameToValue;
		nameToValue.insert(make_pair("F-score",fScore));
		nameToValue.insert(make_pair("TP",truePositives));
		nameToValue.insert(make_pair("TN",trueNegatives));
		nameToValue.insert(make_pair("FP",falsePositives));
		nameToValue.insert(make_pair("FN",falseNegatives));

		return  nameToValue;
} 