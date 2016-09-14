//
//  attacker.cpp
//  pGame
//
//  Created by Samuel Ang on 8/18/16.
//  Copyright (c) 2016 Samuel Ang. All rights reserved.
//

#include "attacker.h"
#include <cmath>
#include <stdlib.h>     /* abs */
#include <string>
#include <iostream>
#include <fstream>
using std::string;
using std::to_string;
using std::ofstream;
int Attacker::getCeling(){
    //shortest path only works for unobstructed rectangular grids currently
    //a more complete version can be written in later
    int firstDif = abs(gameBoard.getAtStart().first - gameBoard.getTrueTargetLoc().first);
    int secDif = abs(gameBoard.getAtStart().second - gameBoard.getTrueTargetLoc().second);
    int distance = (firstDif + secDif);
    int worstCaseShortestPath = (distance * gameBoard.getGuessTax())+(distance * gameBoard.getMoveTax());
    int movesCeling = worstCaseShortestPath/gameBoard.getMoveTax();
    return movesCeling;
}

//creates a variable string for the .lp file. [Example]: "V((0,0),(1,1))
string Attacker::createLpVar(string pre, pair<int, int> assumedTarget, pair<int, int> state){
    string output = "";
    output += pre;
    output += "((" + to_string(assumedTarget.first) + "," + to_string(assumedTarget.second) + "),";
    output += "(" + to_string(state.first) + "," + to_string(state.second) + "))";
    return output;
}

void Attacker::convertToLp(){
    double gTax = gameBoard.getGuessTax();
    double mTax = gameBoard.getMoveTax();
    
    vector<string> coef;
    vector<string> expectedValues;
    
    vector<pair<int,int>> targetLocs = gameBoard.getTargets();
    for(int i = 0; i<targetLocs.size(); ++i){
        //push back coefficient strings
        coef.push_back(to_string((gameBoard.getTargetProb((make_pair(targetLocs.at(i).first,targetLocs.at(i).second))))));
        //push back corresponding variable names for expected payoff
        string variable = createLpVar("V", make_pair(targetLocs.at(i).first, targetLocs.at(i).second),gameBoard.getAtStart());
        expectedValues.push_back(variable);
    }
    //open the file
    ofstream lpFile;
    lpFile.open ("linear_program.lp");
    lpFile << "maximize\n";
    //write coefficients with corresponding variables for expected payoff
    for(int i = 0; i<coef.size(); ++i){
        lpFile << coef.at(i) << expectedValues.at(i);
        if(i<coef.size()-1){
            lpFile << " + ";
        }
    }
    //create constraints
    lpFile << "\n";
    lpFile << "subject to\n";
    int maxMoves = getCeling();
    int moveTax = gameBoard.getMoveTax();
    int guessTax = gameBoard.getGuessTax();
    int trueTargetReward = gameBoard.getTrueTargetReward();
    
    for(int h = 0; h<gameBoard.getHeight(); ++h){
        for(int w = 0; w<gameBoard.getWidth(); ++w){
            for(auto at : gameBoard.getTargets()){
                for(auto ac : gameBoard.getConnectionsOfNode(make_pair(h, w))){
                //write the node we're working on
                    lpFile << createLpVar("V", at, make_pair(h,w));
                    if(!gameBoard.isNodeATarget(make_pair(h, w))){
                        for(auto t : gameBoard.getTargets()){
                            double coef = moveTax;
                            if(t==at){
                                coef+=guessTax;
                            }
                            if(ac==at){
                                coef-=trueTargetReward;
                            }
                            lpFile << " + ";
                            lpFile << to_string(coef);
                            lpFile << createLpVar("F", t, make_pair(h, w));
                        }
                        lpFile << " + ";
                        lpFile << createLpVar("V", at, ac);
                        lpFile << " <= 0\n";
                    }else{
                        lpFile << " = 0\n";
                    }
                }
            }
        }
    }
    
    lpFile << "end";
    lpFile.close();

    }
    

