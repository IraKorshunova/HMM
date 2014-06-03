#pragma once
#include <vector>
#include <string>  
#include <map>
/**
* The <c>Evaluator</c> class has one static method for comparing predicted and actual sequence.
*/
class Evaluator
{

public:
	/**
	* Calculates elements of confusion matrix and F-score given instances of actual and predicted classes and a specified class.
	* @param  predictedSequence predicted class labels of insctances
	* @param  actualSequence	actual class labels of insctances
	* @param  classLabel     	specifies for which class confusion matrix is computed
	* @return map from metrics name (TP, TN, FP, FN, F-score) to its value
	*/
	static std::map<std::string,double> evaluate(const std::vector<std::string>& predictedSequence,
												 const std::vector<std::string>& actualSequence, 
												 const std::string& classLabel); 
};

