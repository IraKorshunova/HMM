#include "StdAfx.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <iterator>
using namespace std;

HiddenMarkovModel::HiddenMarkovModel(const map<pair<string,string>,double>& transitionProbabilities, 
									 const map<pair<string,string>,double>& emissionProbabilities,
									 const map<string,double>& initialProbabilities) {

		typedef map<pair<string,string>,double>::const_iterator probabilityMapIterator;

		// set of states
		for(map<string,double>::const_iterator iter = initialProbabilities.begin(); iter != initialProbabilities.end(); iter++) {
			states.insert(iter->first);
		}

		// set of observations
		for(probabilityMapIterator iter = emissionProbabilities.begin(); iter != emissionProbabilities.end(); iter++) {
			pair<string,string> key =  iter->first;
			observations.insert(key.second);
		}
		
		numberOfStates = states.size();
		numberOfObservations = observations.size();
		
		// map from state to number and reverse 
		int i = 0;
		numberToState.resize(numberOfStates);
		for(set<string>::iterator iter = states.begin(); iter != states.end(); iter++) {
			stateToNumber.insert(make_pair(*iter,i));
			numberToState[i] = *iter;
			i++;
		}

		// map from observation to number and reverse
		i=0;
		numberToObservation.resize(numberOfObservations);
		for(set<string>::iterator iter = observations.begin(); iter != observations.end(); iter++) {
			observationToNumber.insert(make_pair(*iter,i));
			numberToObservation[i] = *iter;
			i++;
		}
	

		/* probability matrices in log scale*/

		// initial probabilities vector
		initialVector.resize(numberOfStates);
		for(map<string,double>::const_iterator iter = initialProbabilities.begin(); iter!= initialProbabilities.end();iter++) {
			initialVector[stateToNumber.find(iter->first)->second] = iter->second;
		} 

		if(isStochastic(initialVector) == false) throw 0;
		
		// initial probabilities to log scale
		for(int i = 0; i < numberOfStates; i++) {
			initialVector[i] = log(initialVector[i]);
		}

		//transition matrix
		transitionMatrix.resize(numberOfStates, vector<double>(numberOfStates)); 
		for(probabilityMapIterator iter = transitionProbabilities.begin(); iter != transitionProbabilities.end(); iter++) {
			pair<string, string> key =  iter->first;
			double value = iter->second;
			int row = stateToNumber.find(key.first)->second;
			int col = stateToNumber.find(key.second)->second;
			transitionMatrix[row][col] = value;
		}

		if(isStochastic(transitionMatrix) == false) throw 1;

		// transition probabilities to log scale
		for(int i = 0; i < numberOfStates; i++) {
			for( int j =0 ;j < numberOfStates; j++){
				transitionMatrix[i][j] = log(transitionMatrix[i][j]);
			}
		}

		//emission matrix
		emissionMatrix.resize(numberOfStates,vector<double>(numberOfObservations));
		for(probabilityMapIterator iter = emissionProbabilities.begin(); iter != emissionProbabilities.end(); iter++){
			pair<string,string> key =  iter->first;
			double value = iter->second;
			int row = stateToNumber.find(key.first)->second;
			int col = observationToNumber.find(key.second)->second;
			emissionMatrix[row][col] = value;
		}

		if(isStochastic(emissionMatrix) == false) throw 2;

		// emission probabilities to log scale
		for(int i = 0; i < numberOfStates; i++) {
			for(int j = 0 ; j < numberOfObservations; j++){
				emissionMatrix[i][j] = log(emissionMatrix[i][j]);
			}
		}
}

HiddenMarkovModel::Track::Track(){
	probabilityOfPath = log(0.0);
}

HiddenMarkovModel::Track::Track(vector<int>& path , double probabilityOfPath){
	this->path = path;
	this->probabilityOfPath = probabilityOfPath;
}


vector<string> HiddenMarkovModel::calculateViterbiPath(const vector<string>& observationLabelsSequence){
	
	// sequence of observations in code numbers
	vector<int> observationSequence;
	for(vector<string>::const_iterator iter = observationLabelsSequence.begin(); iter != observationLabelsSequence.end(); iter++) {
			observationSequence.push_back(observationToNumber.find(*iter)->second);
	}

	vector<Track> stateToTrack; // serves as a map from code number of state (= position in this vector) to its Track

	for(int i = 0; i < numberOfStates; i++) {
		vector<int> path;
		path.push_back(i); 
		stateToTrack.push_back(Track(path, initialVector[i] + emissionMatrix[i][*observationSequence.begin()]));
	}

	for(vector<int>::iterator observation = observationSequence.begin()+1; observation != observationSequence.end(); observation++) {
		vector<Track> tempTracksStorage(numberOfStates);

		for(int  nextState = 0; nextState < numberOfStates; nextState++) {
			Track nextTrack;

			for(int prevState = 0; prevState < numberOfStates; prevState++) {
				Track prevTrack = stateToTrack[prevState];

				prevTrack.probabilityOfPath += emissionMatrix[nextState][*observation] + transitionMatrix[prevState][nextState];

				if(prevTrack.probabilityOfPath > nextTrack.probabilityOfPath) {
					nextTrack.path = prevTrack.path;
					nextTrack.path.push_back(nextState);
					nextTrack.probabilityOfPath = prevTrack.probabilityOfPath;
				}
			}
			tempTracksStorage[nextState] = nextTrack;
		}
		stateToTrack = tempTracksStorage;
	}

	Track finalTrack;

	for(int state = 0; state < numberOfStates; state++) {
		Track track = stateToTrack[state];
		if(track.probabilityOfPath > finalTrack.probabilityOfPath) {
			finalTrack.path = track.path;
			finalTrack.probabilityOfPath = track.probabilityOfPath;
		}
	}

	vector<string> viterbiPath;
	for(vector<int>::iterator iter = finalTrack.path.begin(); iter != finalTrack.path.end(); iter++){
		viterbiPath.push_back(numberToState[*iter]);
	}
	return viterbiPath;
}

vector<string> HiddenMarkovModel::calculateSequenceByForwardBackward(const vector<string>& observationLabelsSequence){
	
	// sequence of observations in code numbers
	vector<int> observationSequence;
	for(vector<string>::const_iterator iter = observationLabelsSequence.begin(); iter != observationLabelsSequence.end(); iter++) {
			observationSequence.push_back(observationToNumber.find(*iter)->second);
	}
	int lengthOfObservationSequence = observationSequence.size();
	
	// forward
	vector<vector<double>> forwardProbabilities; ; // dim = lenghtOfObservationSequence*numberOfSates
	vector<double> firstVector;
	for(int i = 0; i < numberOfStates; i++) {
		firstVector.push_back(initialVector[i] + emissionMatrix[i][observationSequence[0]]); 
	}
	forwardProbabilities.push_back(firstVector); 

	for(int t = 1; t < lengthOfObservationSequence; t++) {
		vector<double> forwardVector;
		for(int j = 0; j < numberOfStates; j++) {
			double logProb = log(0.0); 
			for(int i = 0; i < numberOfStates; i++ ) {
				logProb = logSum(logProb, forwardProbabilities[t-1][i] + transitionMatrix[i][j]); 
			}
			forwardVector.push_back(logProb + emissionMatrix[j][observationSequence[t]]);
		}
		forwardProbabilities.push_back(forwardVector);
	}

	//backward
	vector<vector<double>> backwardProbabilities(lengthOfObservationSequence,vector<double>(numberOfStates));
	backwardProbabilities[lengthOfObservationSequence-1] = vector<double>(numberOfStates);
	for(int t = lengthOfObservationSequence-2; t > 0 ; t --) {
		vector<double> backwardVector;
		for(int i = 0; i < numberOfStates; i++ ) {
			double logProb = log(0.0);
			for(int j = 0; j < numberOfStates; j++) {
				logProb = logSum(logProb, transitionMatrix[i][j] + backwardProbabilities[t+1][j] + emissionMatrix[j][observationSequence[t+1]]);
			}
			backwardVector.push_back(logProb);
		}
		backwardProbabilities[t] = backwardVector;
	}


	// posterior
	vector<vector<double>> posteriorProbabilities(lengthOfObservationSequence,vector<double>(numberOfStates));
	for(int t = 0; t < lengthOfObservationSequence; t++) {
		double normalizingConst = log(0.0);
		for(int i = 0; i < numberOfStates; i++) {
			posteriorProbabilities[t][i] = forwardProbabilities[t][i] + backwardProbabilities[t][i];
			normalizingConst = logSum(normalizingConst, posteriorProbabilities[t][i]);
		}
		for (int i = 0; i < numberOfStates; i++) {
			posteriorProbabilities[t][i] -= normalizingConst;  
		}
	}

	// decoding
	vector<string> sequence; 
	for(int t = 0; t < lengthOfObservationSequence; t++) {
		int stateNumber = distance(posteriorProbabilities[t].begin(), max_element(posteriorProbabilities[t].begin(), posteriorProbabilities[t].end()));
		sequence.push_back(numberToState[stateNumber]);
	}

	return sequence;
}


double HiddenMarkovModel:: logSum(double x, double y) {
	if( !_finite(x) || !_finite(y)) {
		if(!_finite(x))	return y;
		else return x;
	}
	else {
		if(x > y) return  x + log(1 + exp(y - x));
		else return  y + log(1 + exp(x - y));
	}
	return 0;
}

bool HiddenMarkovModel::isStochastic(const vector<double>& probabilityVector) {
	double sum = 0.0;
	for(vector<double>::size_type i = 0; i < probabilityVector.size(); i++) {
		sum += probabilityVector[i];
	}
	if(sum != 1) return false;
	else return true;
}

bool HiddenMarkovModel::isStochastic(const vector<vector<double>>& probabilityMatrix) {
	for(vector<vector<double>>::size_type i= 0; i < probabilityMatrix.size();i++){
		if(!isStochastic(probabilityMatrix[i])) return false;
	}
	return true;
}

HiddenMarkovModel::~HiddenMarkovModel(void) {
}