///@author Krenar Banushi
///@date January 31, 2023
///@brief This program will load in two files (containing district and eligible voter data respectively) in a csv format
///       It then will allow the user to search for a state then return if that state was gerrymandered along with the voting statistics
///       It can also plot the voting percentage per district

#include "ourvector.h"
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
using namespace std;

struct State
{
    string stateName;
    ourvector<int> democraticVotesPerDistrict;
    ourvector<int> republicanVotesPerDistrict;
    int eligibleVoters;
};

//************************FUNCTION PROTOTYPES************************//
void DisplayMainMenu(const bool& dataLoaded, const string& chosenState);
int SearchForState(ourvector<string>& splitInput, string& stateName, ourvector<State>& states);
void PlotDistrictData(const int& stateIndex, ourvector<State>& states);
void PrintStateStats(const int& stateIndex, ourvector<State>& states);
void LoadFiles(bool& dataLoaded, ourvector<string>& fileNames, ourvector<State>& states);
void CreateAndAddState(const string& data, ourvector<State>& states);
void AppendEligibleVoters(string& line, ourvector<State>& states);
int FindStateIndex(string& stateName, ourvector<State>& states);
void SplitDelimiter(string line, ourvector<string>& splitLine, const string& delimiter);
string StringToLower(string& word);
void CalcWastedVotes(State& state, int& wastedDemocrat, int& wastedRepublican, int& totalVotes);

//************************HELPER FUNCTIONS************************//

/// @brief Using a provided string return the lowercase version of the string
/// @param word the provided string to be converted
/// @return the lowercase string
string StringToLower(string& word)
{
    string lowerCaseString;
    for (char letter : word)
    {
        lowerCaseString += tolower(letter);
    }

    return lowerCaseString;
}

/// @brief Using a line we will split it based on a provided delimiter and store it in an ourvector of type string
/// @param line the line to be split and stored
/// @param splitLine the vector we will store the line substrings in to
///                  return by reference
/// @param delimiter user specified delimiter to search for
void SplitDelimiter(string line, ourvector<string>& splitLine, const string& delimiter)
{
    int delimiterPos = int(line.find(delimiter));
    while (delimiterPos >= 0)
    {
        splitLine.push_back(line.substr(0, delimiterPos));
        line = line.substr(delimiterPos + 1, line.size());

        delimiterPos = int(line.find(delimiter));
    }
    splitLine.push_back(line);
}

/// @brief Using a line of data we create a State object and fill it using the provided data and cout the state name and number of districts
/// @param data string line of data provided from the file
/// @param states ourvector of type State to append our new State to
///               return by reference
void CreateAndAddState(const string& data, ourvector<State>& states)
{
    State state;
    ourvector<string> splitData;
    SplitDelimiter(data, splitData, ",");
    int temp;

    state.stateName = splitData.at(0);

    for (uint i = 1; i < splitData.size(); i += 3)
    {
        stringstream ss;
        ss << splitData.at(i + 1) << " " << splitData.at(i + 2);
        ss >> temp;
        state.democraticVotesPerDistrict.push_back(temp);
        ss >> temp;
        state.republicanVotesPerDistrict.push_back(temp);
        ss.str(std::string());
        ss.clear();
    }


    cout << "..." << state.stateName << "..." << state.democraticVotesPerDistrict.size() << " districts total" << endl;

    states.push_back(state);
}

/// @brief Using a provided string of the name of the state we return what index the state is being stored at in the ourvector of type State
/// @param stateName string of the name of the state we are searching for
/// @param states passed by reference ourvector of type State to search in
/// @return return integer of the index where the state was found else return -1 if there was no state found with that name
int FindStateIndex(string& stateName, ourvector<State>& states)
{
    string lowercaseName = StringToLower(stateName);
    for (uint i = 0; i < states.size(); i++)
    {
        if (StringToLower(states.at(i).stateName) == lowercaseName)
            return i;
    }
    return -1;
}

/// @brief Using a line from the eligible voters file we search for the state and add the information to the State object in ourvector
///        and cout the name of the state along with number of eligible voters if there was a cooresponding State already created
/// @param line string of data taken from eligible voters file
/// @param states passed by reference ourvector of type State
///        return by reference
void AppendEligibleVoters(string& line, ourvector<State>& states)
{
    ourvector<string> splitLine;
    SplitDelimiter(line, splitLine, ",");
    int stateIndex = FindStateIndex(splitLine.at(0), states);
    int voters;
    stringstream ss;

    ss << splitLine.at(1);
    ss >> voters;

    if (stateIndex != -1)
    {
        states.at(stateIndex).eligibleVoters = voters;
        cout << "..." << splitLine.at(0) << "..." << voters << " eligible voters" << endl;
    }

    ss.str(std::string());
    ss.clear();
}

/// @brief Given a state we calculate the wasted democratic votes, republican votes, and total votes.
/// @param state State object we want to calculate wasted votes for
/// @param wastedDemocrat passed by reference integer of wasted democratic votes in all districts
/// @param wastedRepublican passed by reference integer of wasted republican votes in all districts
/// @param totalVotes passed by reference integer of the totalVotes in all districts
void CalcWastedVotes(State& state, int& wastedDemocrat, int& wastedRepublican, int& totalVotes)
{
    int overHalf = 0, demVotes = 0, repVotes = 0;
    for (uint i = 0; i < state.democraticVotesPerDistrict.size(); i++)
    {
        demVotes = state.democraticVotesPerDistrict.at(i);
        repVotes = state.republicanVotesPerDistrict.at(i);
        overHalf = ((demVotes + repVotes) / 2) + 1;

        if (state.democraticVotesPerDistrict.at(i) > state.republicanVotesPerDistrict.at(i))
        {
            wastedDemocrat += (demVotes - overHalf);
            wastedRepublican += repVotes;
        }
        else
        {
            wastedDemocrat += demVotes;
            wastedRepublican += (repVotes - overHalf);
        }
        totalVotes += demVotes + repVotes;
    }
}

//************************PRIMARY FUNCTIONS************************//

/// @brief Using file names provided by the user in an ourvector of type string we open open the files and create State objects using each line
/// @param dataLoaded Passed by reference boolean indicating if the files were all able to be opened and loaded
/// @param fileNames ourvector of type string with the user's space seperated inputs
/// @param states ourvector of type State to append a new State to
///               return by reference
void LoadFiles(bool& dataLoaded, ourvector<string>& fileNames, ourvector<State>& states)
{
    ifstream inFS;
    string line;

    for (uint i = 1; i < fileNames.size(); i++)
    {
        inFS.open(fileNames.at(i));

        if (!inFS.is_open())
        {
            if (i == 1)
                cout << "Invalid first file, try again." << endl;
            else if (i == 2)
                cout << "Invalid second file, try again." << endl;

            dataLoaded = false;
            return;
        }

        cout << "Reading: " << fileNames.at(i) << endl;
        while (getline(inFS, line))
        {
            if (i == 1)
                CreateAndAddState(line, states);
            else
                AppendEligibleVoters(line, states);
        }

        dataLoaded = true;
        inFS.close();
    }
}

/// @brief Using the cooresponding state index from ourvector of type State we print the State statistics
/// @param stateIndex integer specifying the index of the state we want to print
/// @param states passed by reference ourvector of type State
void PrintStateStats(const int& stateIndex, ourvector<State>& states)
{
    int wastedDem = 0, wastedRep = 0, totalVotes = 0;
    double efficiencyGap = 0;
    bool isGerrymandered = false;

    CalcWastedVotes(states.at(stateIndex), wastedDem, wastedRep, totalVotes);
    efficiencyGap = (1.0 * (abs(wastedDem - wastedRep)) / totalVotes) * 100;

    isGerrymandered = (efficiencyGap >= 7 && states.at(stateIndex).democraticVotesPerDistrict.size() >= 3) ? true : false;

    cout << "Gerrymandered: " << ((isGerrymandered) ? "Yes" : "No") << endl;
    if (isGerrymandered)
    {
        cout << "Gerrymandered against: " << ((wastedDem > wastedRep) ? "Democrats" : "Republicans") << endl
             << "Efficiency Factor: " << efficiencyGap << "%" << endl;
    }
    cout << "Wasted Democratic votes: " << wastedDem << endl
         << "Wasted Republican votes: " << wastedRep << endl
         << "Eligible voters: " << states.at(stateIndex).eligibleVoters << endl;
}

/// @brief Using the cooresponding state index from ourvector of type State we plot the voter data for each district
/// @param stateIndex integer specifying the index of the state we want to plot
/// @param states passed by reference ourvector of type State 
void PlotDistrictData(const int& stateIndex, ourvector<State>& states)
{
    int demVoterRatio, demVotes, repVotes, remainingRepVotes;

    for (uint i = 0; i < states.at(stateIndex).democraticVotesPerDistrict.size(); i++)
    {
        demVotes = states.at(stateIndex).democraticVotesPerDistrict.at(i);
        repVotes = states.at(stateIndex).republicanVotesPerDistrict.at(i);

        cout << "District: " << i + 1 << endl;

        if ((demVotes + repVotes) > 0)
        {
            demVoterRatio = (100.0 * (demVotes) / (demVotes + repVotes));
            for (int j = 0; j < 100; j++)
            {
                if (demVoterRatio > 0)
                    cout << "D";
                else
                    cout << "R";
                demVoterRatio--;
            }
        }
        cout << endl;
    }
}

/// @brief Display the main menu using a boolean specifying if the data was loaded and string of the name of the chosen state
/// @param dataLoaded boolean variable if the data has been successfully loaded
/// @param chosenState string of the name of the state the user chose
void DisplayMainMenu(const bool& dataLoaded, const string& chosenState)
{
    cout << endl;
    cout << "Data loaded? " << ((dataLoaded) ? "Yes" : "No") << endl
         << "State: " << ((chosenState.size() > 0) ? chosenState : "N/A") << endl
         << endl
         << "Enter command: ";
}

/// @brief Find State index using user input in ourvector of type string and ourvector of type State and return correct state name in string stateName
/// @param splitInput passed by reference ourvector of type string with space seperated user input
/// @param stateName passed by reference string to store state name with correct uppercasing
/// @param states passed by reference ourvector of type State to search for cooresponding index
/// @return index of the State with matching state name from ourvector of type State
///         return by reference state name with correct uppercasing
int SearchForState(ourvector<string>& splitInput, string& stateName, ourvector<State>& states)
{
    string searchQuery;
    //This create one string for states with several words in their name
    for (uint i = 1; i < splitInput.size(); i++)
    {
        searchQuery += splitInput.at(i);
        if (i != splitInput.size() - 1)
            searchQuery += " ";
    }
    int stateIndex = FindStateIndex(searchQuery, states);
    if (stateIndex != -1)
    {
        stateName = states.at(stateIndex).stateName;
        return stateIndex;
    }
    cout << "State does not exist, search again." << endl;

    return stateIndex;
}

/// @brief Recieve user input and call appropriate function based on the first word of their input
/// @return int for compiler purposes
int main()
{
    bool dataLoaded = false;
    ourvector<State> states;
    ourvector<string> splitInput;
    string input, chosenState;
    int stateIndex = -1;

    cout << "Welcome to the Gerrymandering App!" << endl;

    do
    {
        splitInput.clear();
        DisplayMainMenu(dataLoaded, chosenState);
        getline(cin, input);
        cout << endl
             << "-----------------------------" << endl
             << endl;
        SplitDelimiter(input, splitInput, " ");

        if (splitInput.at(0) == "load")
        {
            if (!dataLoaded)
                LoadFiles(dataLoaded, splitInput, states);
            else
                cout << "Already read data in, exit and start over." << endl;
        }
        else if (splitInput.at(0) == "search" && (states.size() > 0))
        {
            stateIndex = SearchForState(splitInput, chosenState, states);
        }
        else if (splitInput.at(0) == "stats" && dataLoaded)
        {
            if (dataLoaded && stateIndex > -1)
                PrintStateStats(stateIndex, states);
            else
                cout << "No state indicated, please search for state first." << endl;
        }
        else if (splitInput.at(0) == "plot" && dataLoaded)
        {
            if (dataLoaded && stateIndex > -1)
                PlotDistrictData(stateIndex, states);
            else
                cout << "errror" << endl;
        }
        else if (!dataLoaded && (splitInput.at(0) != "exit")){
            cout << "No data loaded, please load data first." << endl;
        }
    } while (splitInput.at(0) != "exit");

    return 0;
}