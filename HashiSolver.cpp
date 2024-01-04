// HashiSolver

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include "dirent.h"
#include <stack>
#include <sstream>

using namespace std;

const int ONE_HORIZONTAL = -1;
const int TWO_HORIZONTAL = -2;
const int ONE_VERTICAL = -11;
const int TWO_VERTICAL = -12;

struct Node
{
	bool isIsland()
	{
		return value > 0 ? true : false;
	};

	bool isEmpty()
	{
		return value == 0 ? true : false;
	}

	bool isBridge()
	{
		return value < 0 ? true : false;
	}

	int value;

	bool allBridgesSet = false;

	bool visited = false;
	bool visited_saved = false;
};


struct Board
{
	const static int MAXROWS = 50;
	const static int MAXCOLS = 50;

	Node data[MAXROWS][MAXCOLS];

	int nrows;
	int ncols;
};

struct {
	string name_of_puzzle = "*";
	bool rotate_and_reverse = true;
	int remove_impossible_bridges = 1;
	int remove_short_circuits = 2;
	bool guess_if_needed = true;
	bool debug_with_solution_file = true;
	bool print_board_after_each_iteration = false;
	bool print_board_after_analyzis = true;
	bool print_board_with_colors = true;
	bool print_short_circuits_found = true;
	bool print_successful_guess = true;
	bool print_info_during_guess = false;
	bool print_ambiguous_rectangle = false;
	bool print_board_when_solution_mismatch = true;
	bool print_is_silent = false;
	bool print_tracebuf = false;
} settings;

struct {
	vector<vector<int>> solution;
	bool has_solution_file = false;
} solution;

struct {
	int no_analyzes = 0;
	int no_analyze_ok = 0;
	int no_analyze_fail = 0;
	int no_short_circuits_removed = 0;
	int no_impossible_bridges_removed = 0;
	int no_puzzles_where_guess_occurred = 0;
	vector<string> failed;
} results;

struct AmbiguousRectangle
{
	int row[4] = { 0, 0, 0, 0 };
	int col[4] = { 0, 0, 0, 0 };
};

enum analyzis_result
{
	failed = 1,
	unknown = 2,
	solved = 3
};

//#define TRACE(m, n) tracebuf##m[n]++;
#define TRACE(m, n) //

//#define CHECK_WITH_SOLUTION checkWithSolution(board, row_next, col_next);
#define CHECK_WITH_SOLUTION //

int tracebuf1[100];

void trim_ending_cr(int& nc, string& line);

void read_ini_file();

void parse_path(string path, string& name, string& ext);

void analyze_puzzle(string filename);

bool load_puzzle(string filename, bool solution_file, vector<vector<int>>& puzzle);

bool isPuzzleValid(string filename, vector<vector<int>>& puzzle);

void analyze_with_rotations(vector<vector<int>> puzzle, vector<vector<int>>puzzle_solution, string testname);

int analyze_with_iterations(vector<vector<int>> puzzle, int& noOfGuesses);

int searchAmbigousRectanglesInSolvedPuzzle(Board& board, vector<AmbiguousRectangle>& rectangles);

void removeImpossibleBridges(Board& board, int row, int col, int island, int no_remaining_bridges, int available_bridges[4], int existing_bridges[4], int& noBridgesRemoved);

bool setBridges(Board& board, int row, int col, int island, int no_remaining_bridges, int available_bridges[4], int existing_bridges[4]);

bool analyzeBoard(Board& board, int iteration, bool checkShortCircuits, bool checkImpossibleBridges, bool call_during_guess, int& noShortCircuitsRemoved, int& noImpossibledBridgesRemoved);

analyzis_result find_missing_bridges(Board& board, int level, int row, int col, int direction, int& iteration, int& noOfGuesses);

int island_in_direction(Board& board, int row, int col, int direction, int& no_connected_bridges);

int is_island_available_in_direction(Board& board, int row, int col, int direction, int& no_available_bridges_in_cell, int& row_found, int& col_found);

void find_bridges_in_island(Board& board, int row, int col, int existing_bridges[4], int available_bridges[4], int& no_existing_bridges, int& no_available_bridges);

void setBridgesInOneDirection(Board& board, int row, int col, int number_of_bridges, int direction);

void restoreBridgesInOneDirection(Board& board, int row, int col, int restore, int direction);

void printBoard(Board& board, int row_show = 0, int col_show = 0);

void printPuzzle(vector<vector<int>>& puzzle);

void printSolution();

void initBoard(vector<vector<int>> puzzle, Board& board);

void setSolution(vector<vector<int>> puzzle_solution);

int find_no_bridges_in_island(Board& board, int row, int col);

void findFirstIsland(Board& board, bool require_not_filled, int& row, int& col);

void checkWithSolution(Board& board, int row, int col);

void saveBoard(Board& board, vector<vector<Node>>& saved_board);

bool compareBoards(Board& board, vector<vector<Node>> saved_board);

void restoreBoard(Board& board, vector<vector<Node>> saved_board);

bool areAllBridgesSet(Board& board, bool logErrors);

int remainingBridges(Board& board, bool logErrors);

void swap(int& a, int& b);

int find_total_no_bridges_set(Board& board);

void rotatePuzzle90degree(vector<vector<int>>& puzzle);

void reverseRowsInPuzzle(vector<vector<int>>& puzzle);

bool check_nodes_connected(Board& board, int row, int col, int direction, int level, bool follow_available_bridges, bool follow_existing_bridges);

void removeBridgesCausingShortCircuits(Board& board, int row, int col, int available_bridges[4], int& noInvalidBridgesRemoved);

bool areAllNodesVisited(Board& board);

void clearVisitedNodes(Board& board);

void saveVisitedNodes(Board& board);

void restoreVisitedNodes(Board& board);

int main()
{
	auto start = chrono::high_resolution_clock::now();

	read_ini_file();

	for (int i = 0; i < 100; i++) {
		tracebuf1[i] = 0;
	}

	bool files_analyzed = false;
	DIR* dr;
	struct dirent* en;
	dr = opendir("./puzzles");
	if (dr) {
		while ((en = readdir(dr)) != NULL) {
			if (en->d_type == DT_REG) {
				if (settings.name_of_puzzle == "*" || settings.name_of_puzzle == en->d_name) {
					string name_part;
					string ext_part;
					parse_path(en->d_name, name_part, ext_part);
					if (ext_part == "puz") {
						string path = "./puzzles/";
						path.append(en->d_name);
						analyze_puzzle(path);
						files_analyzed = true;
					}
				}
			}
		}
		closedir(dr);
	}
	else {
		cout << "Error - failed to open directory 'puzzles'" << endl;
	}
	if (!files_analyzed) {
		cout << "Error - No files found" << endl;
	}

	auto stop = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

	cout << endl << "No. analyzes: " << results.no_analyzes << ", solved: " << results.no_analyze_ok << ", failed: " << results.no_analyze_fail << ", duration(ms) : " << duration.count() << endl;
	cout << "No. invalid bridges removed: " << results.no_short_circuits_removed << ", no. impossible bridges removed: " << results.no_impossible_bridges_removed << ", no. puzzles with guessing: " << results.no_puzzles_where_guess_occurred << endl;

	if (results.failed.size() > 0) {
		cout << "Analyzis failed for following puzzles:" << endl;
		for (int i = 0; i < (int)results.failed.size(); i++) {
			cout << results.failed[i] << endl;
		}
	}

	if (settings.print_tracebuf) {
		for (int i = 0; i <= 26; i++) {
			cout << "tracebuf[" << i << "] = " << tracebuf1[i] << endl;
		}
	}

	return 0;
}

// Under Linux lines read from file normally end with just <LF>, not <CR><LF> as files created under Windows
void trim_ending_cr(int& nc, string& line)
{
	const int CR = 13;

	if (nc > 0 && (int)line[nc - 1] == CR) {
		line.pop_back();
		nc--;
	}
}

void parse_path(string path, string& name, string& ext)
{
	bool dot_found = false;
	for (int i = 0; i < (signed)path.length(); i++) {
		if (path[i] == '.' && i > 0) {
			dot_found = true;
		}
		else if (!dot_found) {
			name.append(path.substr(i, 1));
		}
		else {
			ext.append(path.substr(i, 1));
		}
	}
}

void read_ini_file()
{
	string filename = "./hashi_solver.ini";

	fstream file;
	file.open(filename.c_str(), ios::in);
	if (!file) {
		cout << "Error - Could not open " << filename.c_str() << endl;
		return;
	}

	string line;

	while (getline(file, line)) {
		int nc = (int)(line.length());
		if (line[0] == '/') {
			continue;
		}
		else {
			int ncols = 0;
			string settings_name = "";
			string settings_value = "";
			bool equal_sign_found = false;
			for (int i = 0; i < nc; i++) {
				if (line[i] == ' ') {
					continue;
				}
				else if (line[i] == '=') {
					equal_sign_found = true;
				}
				else if (line[i] != '=' && !equal_sign_found) {
					settings_name.append(line.substr(i, 1));
				}
				else if (line[i] != '=' && equal_sign_found) {
					settings_value.append(line.substr(i, 1));
				}
			}

			int nc2 = (int)(settings_value.length());
			trim_ending_cr(nc2, settings_value);

			if (settings_name == "name_of_puzzle") {
				settings.name_of_puzzle = settings_value;
			}
			else if (settings_name == "rotate_and_reverse") {
				settings.rotate_and_reverse = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "removeImpossibleBridges") {
				settings.remove_impossible_bridges = stoi(settings_value);
			}
			else if (settings_name == "removeShortCircuits") {
				settings.remove_short_circuits = stoi(settings_value);
			}
			else if (settings_name == "guess_if_needed") {
				settings.guess_if_needed = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "debug_with_solution_file") {
				settings.debug_with_solution_file = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "print_board_after_each_iteration") {
				settings.print_board_after_each_iteration = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "print_board_after_analyzis") {
				settings.print_board_after_analyzis = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "print_board_with_colors") {
				settings.print_board_with_colors = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "print_short_circuits_found") {
				settings.print_short_circuits_found = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "print_successful_guess") {
				settings.print_successful_guess = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "print_info_during_guess") {
				settings.print_info_during_guess = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "print_ambiguous_rectangle") {
				settings.print_ambiguous_rectangle = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "print_board_when_solution_mismatch") {
				settings.print_board_when_solution_mismatch = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "print_is_silent") {
				settings.print_is_silent = stoi(settings_value) == 1 ? true : false;
			}
			else if (settings_name == "print_tracebuf") {
				settings.print_tracebuf = stoi(settings_value) == 1 ? true : false;
			}
		}
	}
	file.close();
}

bool load_puzzle(string filename, bool solution_file, vector<vector<int>>& puzzle)
{
	int nrows = 0;

	fstream file;
	file.open(filename.c_str(), ios::in);
	if (!file) {
		if (!solution_file) {
			cout << endl << "Puzzle: " << filename << endl;
			cout << "Error - File could not be opened" << endl;
		}
		return false;
	}
	string line;

	// Get the input from the input file until EOF
	while (getline(file, line)) {

		int nc = (int)(line.length());

		trim_ending_cr(nc, line);

		while (nc > 0) {
			if (line[nc - 1] == ' ' || line[nc - 1] == '\t') {
				nc--;
			}
			else {
				break;
			}
		}

		if (nc == 0 || line[0] == '/') {
			continue;
		}
		else {
			puzzle.resize(nrows + 1);
			int ncols = 0;
			string str_value = "";
			for (int i = 0; i < nc; i++) {
				if (line[i] == ' ' || line[i] == '\t') {
					continue;
				}
				else if (line[i] == '-') {
					str_value.append("-");
				}
				else if (line[i] >= '0' && line[i] <= '9') {
					str_value.append(line.substr(i, 1));
				}
				else if (line[i] != ',') {
					cout << endl << "Puzzle: " << filename << endl;
					cout << " Error - illegal character: " << line[i] << endl;
					return false;
				}
				if (line[i] == ',' || i == nc - 1) {
					int val = stoi(str_value);
					puzzle[nrows].resize(ncols + 1);
					puzzle[nrows][ncols] = val;
					ncols++;
					str_value = "";
				}

			}
			if (ncols > 0) {
				if (nrows > 0 && puzzle[nrows].size() != puzzle[nrows - 1].size()) {
					cout << "Error: Puzzle must have same length of each row!" << endl;
					return false;
				}
				nrows++;
			}
		}
	}
	file.close();

	return true;
}

bool isPuzzleValid(string filename, vector<vector<int>>& puzzle)
{
	int nrows = (int)(puzzle.size());
	int ncols = (int)(puzzle[0].size());

	if (nrows < 3 || ncols < 3) {
		cout << endl << "Puzzle: " << filename << endl;
		cout << " Error - too few rows and/or columns" << endl;
		return false;
	}

	bool empty = true;
	int no_odd_islands = 0;
	for (int row = 0; row < nrows; row++)
	{
		for (int col = 0; col < ncols; col++)
		{
			int island = puzzle[row][col];
			if (island > 0) {
				empty = false;
				if (1 == island % 2) {
					no_odd_islands++;
				}
			}
		}
	}

	if (empty) {
		cout << endl << "Puzzle: " << filename << endl;
		cout << " Error - puzzle is empty" << endl;
		return false;
	}

	if (1 == no_odd_islands % 2) {
		cout << endl << "Puzzle: " << filename << endl;
		cout << " Error - Puzzle has an odd number of odd-numbered islands. Thus lacks solution!" << endl;
		return false;
	}

	vector<int> rows;
	vector<int> cols;

	rows.push_back(0);
	cols.push_back(0);

	rows.push_back(0);
	cols.push_back(ncols - 2);

	rows.push_back(nrows - 2);
	cols.push_back(0);

	rows.push_back(nrows - 2);
	cols.push_back(ncols - 2);

	for (int corner = 0; corner < 4; corner++) {
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				int row = rows[corner] + i;
				int col = cols[corner] + j;
				int island = puzzle[row][col];
				if (island > 4) {
					cout << endl << "Puzzle: " << filename << endl;
					cout << " Error - island " << island << " is too close to corner" << endl;
					return false;
				}
			}
		}
	}

	rows.clear();
	rows.push_back(0);
	rows.push_back(1);
	rows.push_back(nrows - 2);
	rows.push_back(nrows - 1);

	for (int i = 0; i < 4; i++) {
		int row = rows[i];
		for (int col = 0; col < ncols; col++) {
			int island = puzzle[row][col];
			if (island > 6) {
				cout << endl << "Puzzle: " << filename << endl;
				cout << " Error - island " << island << " is too close to edge" << endl;
				return false;
			}
		}
	}

	cols.clear();
	cols.push_back(0);
	cols.push_back(1);
	cols.push_back(ncols - 2);
	cols.push_back(ncols - 1);

	for (int i = 0; i < 4; i++) {
		int col = cols[i];
		for (int row = 0; row < nrows; row++) {
			int island = puzzle[row][col];
			if (island > 6) {
				cout << endl << "Puzzle: " << filename << endl;
				cout << " Error - island " << island << " is too close to edge" << endl;
				return false;
			}
		}
	}

	for (int row = 0; row < nrows - 1; row++)
	{
		for (int col = 0; col < ncols - 1; col++)
		{
			int island = puzzle[row][col];

			if (island == 0) {
				continue;
			}

			int island2 = puzzle[row][col + 1];
			if (island > 0 && island2 > 0) {
				cout << endl << "Puzzle: " << filename << endl;
				cout << " Error - Islands " << island << " and " << island2 << " are too close" << endl;
				return false;
			}

			int island3 = puzzle[row + 1][col];
			if (island > 0 && island3 > 0) {
				cout << endl << "Puzzle: " << filename << endl;
				cout << " Error - Islands " << island << " and " << island3 << " are too close" << endl;
				return false;
			}

			bool isolated = true;
			for (int i = 0; i < ncols; i++) {
				if (i != col && puzzle[row][i] > 0) {
					isolated = false;
					break;
				}
			}

			for (int i = 0; i < nrows; i++) {
				if (i != row && puzzle[i][col] > 0) {
					isolated = false;
					break;
				}
			}

			if (isolated) {
				cout << endl << "Puzzle: " << filename << endl;
				cout << " Error - Island " << island << " at (" << row << ", " << col << ") is isolated" << endl;
				return false;
			}
		}

	}

	return true;
}

void analyze_puzzle(string filename)
{
	vector<vector<int>> puzzle;
	vector<vector<int>> puzzle_solution;

	bool have_puzzle = load_puzzle(filename, false, puzzle);

	if (!have_puzzle) {
		return;
	}

	bool isValid = isPuzzleValid(filename, puzzle);

	if (!isValid) {
		results.no_analyzes++;
		results.no_analyze_fail++;
		results.failed.push_back(filename);
		return;
	}

	if (settings.debug_with_solution_file) {
		string name_part;
		string ext_part;
		parse_path(filename, name_part, ext_part);

		string solution_file_name = name_part;
		solution_file_name.append(".sol");

		bool have_puzzle_solution = load_puzzle(solution_file_name, true, puzzle_solution);

	}

	analyze_with_rotations(puzzle, puzzle_solution, filename);
}

void analyze_with_rotations(vector<vector<int>> puzzle, vector<vector<int>> puzzle_solution, string testname)
{
	string degrees[4] = { "  0", " 90", "180", "270" };

	solution.has_solution_file = false;

	int noOfGuesses;

	if (settings.debug_with_solution_file && puzzle_solution.size() > 0) {
		solution.has_solution_file = true;
		setSolution(puzzle_solution);
	}

	if (!settings.print_is_silent) {
		cout << endl << "Puzzle: " << testname << endl;
	}

	for (int i = 0; i < 4; i++) {
		if (settings.rotate_and_reverse) {
			if (!settings.print_is_silent) {
				cout << "	Rotated " << degrees[i] << " degrees:";
			}
		}

		int res = analyze_with_iterations(puzzle, noOfGuesses);
		results.no_analyzes++;
		//res > 0 ? results.no_analyze_ok++ : results.no_analyze_fail++;

		if (res > 0) {
			results.no_analyze_ok++;
		}
		else {
			results.no_analyze_fail++;
			results.failed.push_back(testname);
		}


		if (noOfGuesses > 0) {
			results.no_puzzles_where_guess_occurred++;
		}

		if (!settings.rotate_and_reverse) {
			break;
		}

		reverseRowsInPuzzle(puzzle);
		if (solution.has_solution_file) {
			reverseRowsInPuzzle(puzzle_solution);
			setSolution(puzzle_solution);
		}

		if (!settings.print_is_silent) {
			cout << "	Reversed rows:      ";
		}
		res = analyze_with_iterations(puzzle, noOfGuesses);
		results.no_analyzes++;
		res > 0 ? results.no_analyze_ok++ : results.no_analyze_fail++;
		if (noOfGuesses > 0) {
			results.no_puzzles_where_guess_occurred++;
		}

		reverseRowsInPuzzle(puzzle);
		if (solution.has_solution_file) {
			reverseRowsInPuzzle(puzzle_solution);
			setSolution(puzzle_solution);
		}

		rotatePuzzle90degree(puzzle);
		if (solution.has_solution_file) {
			rotatePuzzle90degree(puzzle_solution);
			setSolution(puzzle_solution);
		}

	}
}

int analyze_with_iterations(vector<vector<int>> puzzle, int& noOfGuesses)
{
	Board board;

	board.nrows = (int)(puzzle.size()) + 2;
	board.ncols = (int)(puzzle[0].size()) + 2;

	initBoard(puzzle, board);

	int iteration = 0;

	int last_bridges_set = 0;
	int no_bridges_added = 0;

	int no_fixed_short_circuits = 0;
	int no_fixed_impossible_bridges = 0;
	int noShortCircuitsRemoved;
	int noImpossibledBridgesRemoved;

	noOfGuesses = 0;

	bool allBridgesSet = false;
	bool singleNetwork = false;
	bool isSolved = false;
	int step = 1;

	int levelsToGuess = 0;

	do
	{
		if (step < 2) {
			iteration++;

			bool checkShortCircuits = (settings.remove_short_circuits == 1 || settings.remove_short_circuits == 3);
			bool checkImpossibleBridges = (settings.remove_impossible_bridges == 1 || settings.remove_impossible_bridges == 3);
			bool status = analyzeBoard(board, iteration, checkShortCircuits, checkImpossibleBridges, false, noShortCircuitsRemoved, noImpossibledBridgesRemoved);
			if (!status) {
				break;
			}

			no_fixed_short_circuits += noShortCircuitsRemoved;
			results.no_short_circuits_removed += noShortCircuitsRemoved;

			no_fixed_impossible_bridges += noImpossibledBridgesRemoved;
			results.no_impossible_bridges_removed += noImpossibledBridgesRemoved;

			int total_no_bridges_set = find_total_no_bridges_set(board);

			no_bridges_added = total_no_bridges_set - last_bridges_set;
			last_bridges_set = total_no_bridges_set;

			if (!settings.print_is_silent && settings.print_board_after_each_iteration && no_bridges_added > 0) {
				cout << " Iteration: " << iteration << "  No bridges added: " << no_bridges_added << endl << endl;
				printBoard(board);
				cout << "-------------------" << endl;
			}
		}
		else {
			if (settings.guess_if_needed) {
				find_missing_bridges(board, 0, -1, -1, 0, iteration, noOfGuesses);
			}
		}

		allBridgesSet = areAllBridgesSet(board, false);

		singleNetwork = check_nodes_connected(board, -1, -1, 0, 0, false, true);

		isSolved = allBridgesSet && singleNetwork;

		if (no_bridges_added == 0 && !isSolved) {
			step++;
		}
	} while (!isSolved && step <= 2);

	int status;

	if (isSolved) {
		if (!settings.print_is_silent) {
			cout << " Solved, after " << (iteration == 1 ? iteration : iteration - 1) << " iterations, " << no_fixed_short_circuits << " short circuits fixed, " << no_fixed_impossible_bridges << " impossible bridges fixed, and " << noOfGuesses << " guesses";
			if (levelsToGuess > 0) {
				cout << " at level " << levelsToGuess << "." << endl;
			}

			vector<AmbiguousRectangle> rectangles;
			searchAmbigousRectanglesInSolvedPuzzle(board, rectangles);

			cout << endl;

			if (settings.print_board_after_analyzis && !(settings.print_board_after_each_iteration && noOfGuesses == 0)) {
				printBoard(board);
			}
		}
		status = 1;
	}
	else {
		if (!settings.print_is_silent) {
			cout << " NOT solved!, after " << (iteration == 1 ? iteration : iteration - 1) << " iterations, " << no_fixed_short_circuits << " short circuits fixed, and " << noOfGuesses << " guesses";
			int remaining_bridges = remainingBridges(board, false);
			if (remaining_bridges > 0) {
				cout << ". " << remaining_bridges << " bridges are missing" << endl;
			}
			else {
				cout << ". Not a single network." << endl;
			}

			if (settings.print_board_after_analyzis && !(settings.print_board_after_each_iteration && noOfGuesses == 0)) {
				printBoard(board);
				printSolution();
			}
		}
		status = -1;
	}

	return status;
}

int searchAmbigousRectanglesInSolvedPuzzle(Board& board, vector<AmbiguousRectangle>& rectangles)
{
	AmbiguousRectangle corners;

	int row_directions[4] = { 0,  1,  0, -1 };
	int col_directions[4] = { 1,  0, -1,  0 };

	int bridges[4] = { ONE_HORIZONTAL, ONE_VERTICAL, ONE_HORIZONTAL, ONE_VERTICAL };
	int crossing_bridges[4] = { ONE_VERTICAL, ONE_HORIZONTAL, ONE_VERTICAL, ONE_HORIZONTAL };

	for (int row = 1; row < board.nrows - 3; row++) {
		for (int col = 1; col < board.ncols - 3; col++) {

			if (board.data[row][col].value > 0) {

				int row_next = row;
				int col_next = col;

				bool found = true;

				corners.row[0] = row_next;
				corners.col[0] = col_next;

				int no_corners = 1;

				int dir_ix;
				for (dir_ix = 0; dir_ix < 4; dir_ix++) {
					int cell;
					int bridge = bridges[dir_ix];
					int crossing_bridge = crossing_bridges[dir_ix];

					do {
						row_next = row_next + row_directions[dir_ix];
						col_next = col_next + col_directions[dir_ix];

						cell = board.data[row_next][col_next].value;

					} while (cell == 0 || cell == bridge || cell == bridge - 1);

					if (cell == crossing_bridge || cell == crossing_bridge - 1) {
						found = false;
						break;
					}

					if (dir_ix < 3) {
						corners.row[dir_ix + 1] = row_next;
						corners.col[dir_ix + 1] = col_next;
						no_corners++;
					}
				}

				if (no_corners == 4) {

					if (row_next != corners.row[0] || col_next != corners.col[0]) {
						found = false;
					}

					int row1 = corners.row[0];
					int col1 = corners.col[0] + 1;

					int row2 = corners.row[0] + 1;
					int col2 = corners.col[0];

					int row3 = corners.row[2];
					int col3 = corners.col[2] - 1;

					int row4 = corners.row[2] - 1;
					int col4 = corners.col[2];

					if (board.data[row1][col1].value == TWO_HORIZONTAL && board.data[row2][col2].value == TWO_VERTICAL) {
						found = false;
					}
					if (board.data[row1][col1].value == TWO_HORIZONTAL && board.data[row3][col3].value == 0) {
						found = false;
					}

					if (board.data[row2][col2].value == TWO_VERTICAL && board.data[row4][col4].value == 0) {
						found = false;
					}

					if (board.data[row1][col1].value == TWO_HORIZONTAL && board.data[row4][col4].value == TWO_VERTICAL) {
						found = false;
					}


					if (board.data[row3][col3].value == TWO_HORIZONTAL && board.data[row2][col2].value == TWO_VERTICAL) {
						found = false;
					}

					if (board.data[row3][col3].value == TWO_HORIZONTAL && board.data[row4][col4].value == TWO_VERTICAL) {
						found = false;
					}
					if (board.data[row3][col3].value == TWO_HORIZONTAL && board.data[row1][col1].value == 0) {
						found = false;
					}

					if (board.data[row4][col4].value == TWO_VERTICAL && board.data[row2][col2].value == 0) {
						found = false;
					}

					if (board.data[row1][col1].value == 0 && board.data[row2][col2].value == 0) {
						found = false;
					}
					if (board.data[row1][col1].value == 0 && board.data[row4][col4].value == 0) {
						found = false;
					}
					if (board.data[row3][col3].value == 0 && board.data[row2][col2].value == 0) {
						found = false;
					}
					if (board.data[row3][col3].value == 0 && board.data[row4][col4].value == 0) {
						found = false;
					}


					if (found) {


						bool both_horizontal_sides_have_bridges = (board.data[row1][col1].value < 0 && board.data[row3][col3].value < 0);
						bool both_vertical_sides_have_bridges = (board.data[row2][col2].value < 0 && board.data[row4][col4].value < 0);

						bool subtract_horizontal_bridges = false;
						bool subtract_vertical_bridges = false;

						if (board.data[row1][col1].value == TWO_HORIZONTAL || board.data[row3][col3].value == TWO_HORIZONTAL) {
							subtract_horizontal_bridges = true;
						}
						else if (board.data[row2][col2].value == TWO_VERTICAL || board.data[row4][col4].value == TWO_VERTICAL) {
							subtract_vertical_bridges = true;
						}
						else if ((board.data[row1][col1].value == ONE_HORIZONTAL || board.data[row3][col3].value == ONE_HORIZONTAL) && both_horizontal_sides_have_bridges) {
							subtract_horizontal_bridges = true;
						}
						else if ((board.data[row2][col2].value == ONE_VERTICAL || board.data[row4][col4].value == ONE_VERTICAL) && both_vertical_sides_have_bridges) {
							subtract_vertical_bridges = true;
						}
						if (subtract_horizontal_bridges || subtract_vertical_bridges) {

							vector<vector<Node>> saved_board(board.nrows, vector<Node>(board.ncols));
							saveBoard(board, saved_board);

							int increment = 1;
							if (subtract_vertical_bridges) {
								increment = -1;
							}

							int row_next = corners.row[0];
							int col_next = corners.col[0];
							while (board.data[row_next][++col_next].value <= 0) {
								board.data[row_next][col_next].value += increment;
							}

							row_next = corners.row[1];
							col_next = corners.col[1];
							while (board.data[++row_next][col_next].value <= 0) {
								board.data[row_next][col_next].value -= increment;
								if (board.data[row_next][col_next].value == ONE_HORIZONTAL) {
									board.data[row_next][col_next].value = ONE_VERTICAL;
								}
								else if (board.data[row_next][col_next].value == ONE_VERTICAL + 1) {
									board.data[row_next][col_next].value = 0;
								}
							}

							row_next = corners.row[2];
							col_next = corners.col[2];
							while (board.data[row_next][--col_next].value <= 0) {
								board.data[row_next][col_next].value += increment;
							}

							row_next = corners.row[3];
							col_next = corners.col[3];
							while (board.data[--row_next][col_next].value <= 0) {
								board.data[row_next][col_next].value -= increment;
								if (board.data[row_next][col_next].value == ONE_HORIZONTAL) {
									board.data[row_next][col_next].value = ONE_VERTICAL;
								}
								else if (board.data[row_next][col_next].value == ONE_VERTICAL + 1) {
									board.data[row_next][col_next].value = 0;
								}
							}

							bool allBridgesSet = areAllBridgesSet(board, false);

							bool singleNetwork = check_nodes_connected(board, -1, -1, 0, 0, false, true);

							if (allBridgesSet && singleNetwork) {
								rectangles.push_back(corners);
								if (settings.print_ambiguous_rectangle) {
									cout << " Ambiguous rectangle found in solved puzzle. Upper left corner=(" << corners.row[0] << ", " << corners.col[0] << ")" << endl;
								}
							}

							restoreBoard(board, saved_board);
						}


					}
				}

			}
		}
	}

	return (int)rectangles.size();
}

bool analyzeBoard(Board& board, int iteration, bool checkShortCircuits, bool checkImpossibleBridges, bool call_during_guess, int& noShortCircuitsRemoved, int& noImpossibledBridgesRemoved)
{
	int row_directions[4] = { 0, -1, 0, 1 };
	int col_directions[4] = { 1, 0, -1, 0 };

	noShortCircuitsRemoved = 0;
	noImpossibledBridgesRemoved = 0;

	for (int row = 1; row < board.nrows - 1; row++) {
		for (int col = 1; col < board.ncols - 1; col++) {

			if (board.data[row][col].isIsland() && !board.data[row][col].allBridgesSet) {

				int existing_bridges[4];
				int available_bridges[4];
				int no_existing_bridges;
				int no_available_bridges;
				find_bridges_in_island(board, row, col, existing_bridges, available_bridges, no_existing_bridges, no_available_bridges);
				int no_remaining_bridges = board.data[row][col].value - no_existing_bridges;

				if (no_remaining_bridges == 0) {
					board.data[row][col].allBridgesSet = true;
				}
				else {
					if (checkShortCircuits) {

						int oldBridges[4];			// only for debugging
						for (int i = 0; i < 4; i++) {
							oldBridges[i] = available_bridges[i];
						}

						int noInvalidBridgesRemoved;
						removeBridgesCausingShortCircuits(board, row, col, available_bridges, noInvalidBridgesRemoved);
						if (noInvalidBridgesRemoved > 0) {
							if (settings.print_short_circuits_found && !settings.print_is_silent) {
								cout << "Removed " << noInvalidBridgesRemoved << " bridges at (" << row << ", " << col << ")" << endl;
							}
							noShortCircuitsRemoved += noInvalidBridgesRemoved;
						}

					}

					if (checkImpossibleBridges) {
						int noBridgesRemoved;
						removeImpossibleBridges(board, row, col, board.data[row][col].value, no_remaining_bridges, available_bridges, existing_bridges, noBridgesRemoved);
						noImpossibledBridgesRemoved += noBridgesRemoved;
					}

					if (!setBridges(board, row, col, board.data[row][col].value, no_remaining_bridges, available_bridges, existing_bridges)) {
						if (!call_during_guess) {
							cout << "Error : Failed to set bridge(s) at row=" << row << ", col=" << col << ". Check if puzzle definition is correct!" << endl;
						}
						return false;
					}

				}
			}

		}

	}
	return true;
}

void removeImpossibleBridges(Board& board, int row, int col, int island, int no_remaining_bridges, int available_bridges[4], int existing_bridges[4], int& noBridgesRemoved)
{
	noBridgesRemoved = 0;

	switch (island) {
	case 1: {
		for (int dir = 0; dir < 4; dir++) {
			if (available_bridges[dir] == 1) {
				int no_connected_bridges;
				int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
				if (cell == 1) {
					available_bridges[dir] = 0;
					noBridgesRemoved++;
					TRACE(1, 0)
				}
			}
		}
		break;
	}
	case 2: {
		if (no_remaining_bridges == 2) {
			for (int dir = 0; dir < 4; dir++) {
				if (available_bridges[dir] == 2) {
					int no_connected_bridges;
					int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
					if (cell == 2) {
						available_bridges[dir]--;
						noBridgesRemoved++;
						TRACE(1, 1)
					}
				}
			}
		}
		else {	// no_remaining_bridges == 1
			int no_connected_1_cells = 0;
			for (int dir = 0; dir < 4; dir++) {
				if (existing_bridges[dir] == 1) {
					int no_connected_bridges;
					int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
					if (cell == 1) {
						no_connected_1_cells++;
					}
				}
			}
			if (no_connected_1_cells == 1) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 1) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 1) {
							available_bridges[dir] = 0;
							noBridgesRemoved++;
							TRACE(1, 2)
						}
					}
				}
			}
		}
		break;
	}
	case 3: {
		if (no_remaining_bridges == 1) {
			int no_connected_1_cells = 0;
			int no_double_connected_2_cells = 0;
			for (int dir = 0; dir < 4; dir++) {
				if (existing_bridges[dir] == 1) {
					int no_connected_bridges;
					int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
					if (cell == 1) {
						no_connected_1_cells++;
					}
				}
				else if (existing_bridges[dir] == 2) {
					int no_connected_bridges;
					int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
					if (cell == 2) {
						no_double_connected_2_cells++;
					}
				}
			}
			if (no_connected_1_cells == 2) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 1) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 1) {
							available_bridges[dir]--;
							noBridgesRemoved++;
							TRACE(1, 3)
						}
					}
				}
			}
			else if (no_connected_1_cells == 1) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 2) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 2) {
							available_bridges[dir]--;
							noBridgesRemoved++;
							TRACE(1, 4)
						}
					}
				}
			}
			else if (no_double_connected_2_cells == 1) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 1) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 1) {
							available_bridges[dir]--;
							noBridgesRemoved++;
							TRACE(1, 5)
						}
					}
				}
			}
		}
		break;
	}
	case 4: {
		if (no_remaining_bridges == 1 || no_remaining_bridges == 2) {
			int no_connected_1_cells = 0;
			int no_double_connected_2_cells = 0;
			for (int dir = 0; dir < 4; dir++) {
				if (existing_bridges[dir] == 1) {
					int no_connected_bridges;
					int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
					if (cell == 1) {
						no_connected_1_cells++;
					}
				}
				else if (existing_bridges[dir] == 2) {
					int no_connected_bridges;
					int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
					if (cell == 2) {
						no_double_connected_2_cells++;
					}
				}
			}
			if (no_double_connected_2_cells == 1 && no_connected_1_cells == 1) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 1) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 1) {
							available_bridges[dir]--;
							noBridgesRemoved++;
							TRACE(1, 6)
						}
					}
				}
			}
			else if (no_double_connected_2_cells == 1 || no_connected_1_cells == 2) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 2) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 2) {
							available_bridges[dir]--;
							noBridgesRemoved++;
							TRACE(1, 7)
						}
					}
				}
			}
		}
		break;
	}
	case 5: {
		if (no_remaining_bridges == 1 || no_remaining_bridges == 2) {
			int no_connected_1_cells = 0;
			int no_double_connected_2_cells = 0;
			for (int dir = 0; dir < 4; dir++) {
				if (existing_bridges[dir] == 1) {
					int no_connected_bridges;
					int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
					if (cell == 1) {
						no_connected_1_cells++;
					}
				}
				else if (existing_bridges[dir] == 2) {
					int no_connected_bridges;
					int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
					if (cell == 2) {
						no_double_connected_2_cells++;
					}
				}
			}
			if (no_double_connected_2_cells == 2) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 1) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 1) {
							available_bridges[dir]--;
							noBridgesRemoved++;
							TRACE(1, 8)
						}
					}
				}
			}
			else if (no_double_connected_2_cells == 1 && no_connected_1_cells == 1) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 2) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 2) {
							available_bridges[dir]--;
							noBridgesRemoved++;
							TRACE(1, 9)
						}
					}
				}
			}
		}
		break;
	}
	case 6: {
		if (no_remaining_bridges == 2) {
			int no_double_connected_2_cells = 0;
			for (int dir = 0; dir < 4; dir++) {
				if (existing_bridges[dir] == 2) {
					int no_connected_bridges;
					int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
					if (cell == 2) {
						no_double_connected_2_cells++;
					}
				}
			}
			if (no_double_connected_2_cells == 2) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 2) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 2) {
							available_bridges[dir]--;
							noBridgesRemoved++;
							TRACE(1, 10)
						}
					}
				}
			}
		}
		break;
	}
	default: {
		break;
	}
	}
}

bool setBridges(Board& board, int row, int col, int island, int no_remaining_bridges, int available_bridges[4], int existing_bridges[4])
{

	if (island == 4) {
		int no_TWO_cell = 0;
		int no_not_TWO_cell = 0;
		int dir_not_TWO_cell = -1;
		for (int dir = 0; dir < 4; dir++) {
			//cout << "dir: " << dir;
			if (existing_bridges[dir] == 2 ||
				available_bridges[dir] == 2 ||
				(existing_bridges[dir] == 1 && available_bridges[dir] == 1)) {
				int no_connected_bridges;
				int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
				if (cell == 2) {
					no_TWO_cell++;
					//cout << "  no_TWO_cell: " << no_TWO_cell;
				}
				else if (available_bridges[dir] > 0) {
					no_not_TWO_cell++;
					dir_not_TWO_cell = dir;
				}
				else if (existing_bridges[dir] > 0) {
					no_not_TWO_cell++;
				}
			}
			else if (available_bridges[dir] > 0) {
				no_not_TWO_cell++;
				dir_not_TWO_cell = dir;
				//cout << "  dir_not_TWO_cell: " << dir_not_TWO_cell;
			}
			else if (existing_bridges[dir] > 0) {
				no_not_TWO_cell++;
			}

			//cout << endl;
		}
		if (no_TWO_cell == 2 && no_not_TWO_cell == 1 && dir_not_TWO_cell > -1 && existing_bridges[dir_not_TWO_cell] == 0) {
			//cout << "----------- case 4 - 2 - 2 ------------------------" << endl;
			//printBoard(board, row, col);
			setBridgesInOneDirection(board, row, col, 1, dir_not_TWO_cell);
			available_bridges[dir_not_TWO_cell]--;
			no_remaining_bridges--;

			TRACE(1, 26)
		}

	}

	int no_available_directions = 0;
	for (int dir = 0; dir < 4; dir++) {
		if (available_bridges[dir] > 0) {
			no_available_directions++;
		}
	}

	int no_available_bridges = 0;
	for (int dir = 0; dir < 4; dir++) {
		no_available_bridges += available_bridges[dir];
	}

	if (no_available_bridges < no_remaining_bridges) {
		//cout << "no_available_bridges: " << no_available_bridges << ", no_remaining_bridges: " << no_remaining_bridges << endl;
		return false;
	}

	if (no_remaining_bridges == no_available_bridges) {
		for (int dir = 0; dir < 4; dir++) {
			if (available_bridges[dir] == 2) {
				setBridgesInOneDirection(board, row, col, 2, dir);
				TRACE(1, 11)
			}
			else if (available_bridges[dir] == 1) {
				if (existing_bridges[dir] == 1) {
					setBridgesInOneDirection(board, row, col, 2, dir);
					TRACE(1, 12)
				}
				else {
					setBridgesInOneDirection(board, row, col, 1, dir);
					TRACE(1, 13)
				}
			}
		}
	}
	else {
		switch (no_available_directions) {
		case 1: {
			for (int dir = 0; dir < 4; dir++) {
				if (available_bridges[dir] > 0) {
					setBridgesInOneDirection(board, row, col, 1, dir);
					TRACE(1, 14)
				}
			}
			break;
		}
		case 2: {
			if (no_remaining_bridges == 1 && no_available_bridges > 1 && island == 2) {
				int existing_no_ONE_cell = 0;
				int available_no_ONE_cell = 0;
				int dir_not_ONE_cell = -1;
				for (int dir = 0; dir < 4; dir++) {
					if (existing_bridges[dir] == 1) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 1) {
							existing_no_ONE_cell++;
						}
					}
					if (available_bridges[dir] == 1) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 1) {
							available_no_ONE_cell++;
						}
						else {
							dir_not_ONE_cell = dir;
						}
					}
					else if (available_bridges[dir] > 1) {
						dir_not_ONE_cell = dir;
					}
				}
				if (existing_no_ONE_cell == 1 && available_no_ONE_cell == 1) {
					setBridgesInOneDirection(board, row, col, 1, dir_not_ONE_cell);
					TRACE(1, 15)
				}
			}
			else if (no_remaining_bridges == 2 && no_available_bridges == 3) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 2) {
						setBridgesInOneDirection(board, row, col, 1, dir);
						TRACE(1, 16)
					}
				}
			}
			else if (no_remaining_bridges == 2 && no_available_bridges == 4 && island == 2) {
				int no_TWO_cell = 0;
				int dir_not_TWO_cell = -1;
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 1) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 2) {
							no_TWO_cell++;
						}
					}
					else if (available_bridges[dir] > 1) {
						dir_not_TWO_cell = dir;
					}
				}
				if (no_TWO_cell == 1) {
					setBridgesInOneDirection(board, row, col, 1, dir_not_TWO_cell);
					TRACE(1, 17)
				}
				else if (no_TWO_cell == 2) {
					setBridgesInOneDirection(board, row, col, 1, dir_not_TWO_cell);
					TRACE(1, 17)
				}
			}
			else if (no_remaining_bridges == 3 && no_available_bridges == 4) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] > 0) {
						setBridgesInOneDirection(board, row, col, 1, dir);
						TRACE(1, 18)
					}
				}
			}
			break;
		}
		case 3: {
			if (no_remaining_bridges == 2 && no_available_bridges >= 3 && island == 2) {
				int no_ONE_cell = 0;
				int dir_not_ONE_cell = -1;
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 1) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 1) {
							no_ONE_cell++;
						}
						else {
							dir_not_ONE_cell = dir;
						}
					}
					else if (available_bridges[dir] > 0) {
						dir_not_ONE_cell = dir;
					}
				}
				if (no_ONE_cell == 2) {
					setBridgesInOneDirection(board, row, col, 1, dir_not_ONE_cell);
					TRACE(1, 19)
				}
			}
			else if ((no_remaining_bridges == 3 && no_available_bridges == 4) ||
				(no_remaining_bridges == 4 && no_available_bridges == 5)) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 2) {
						setBridgesInOneDirection(board, row, col, 1, dir);
						TRACE(1, 20)
					}
				}
			}
			else if (island == 4 && no_remaining_bridges == 4 && (no_available_bridges == 5 || no_available_bridges == 6)) {
				int no_ONE_cell = 0;
				int no_TWO_cell = 0;
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 1) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 1) {
							no_ONE_cell++;
						}
					}
					else if (available_bridges[dir] == 2) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 2) {
							no_TWO_cell++;
						}
					}

				}
				if (no_TWO_cell == 3 || (no_TWO_cell == 2 && no_ONE_cell == 1)) {
					for (int dir = 0; dir < 4; dir++) {
						if (available_bridges[dir] > 0) {
							setBridgesInOneDirection(board, row, col, 1, dir);
							TRACE(1, 25)
						}
					}
				}
			}
			else if (no_remaining_bridges == 5 && no_available_bridges == 6) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] > 0) {
						setBridgesInOneDirection(board, row, col, 1, dir);
						TRACE(1, 21)
					}
				}
				break;
			}
		}
		case 4: {
			if (no_remaining_bridges == 3 && no_available_bridges >= 4 && island == 3) {
				int no_ONE_cell = 0;
				int dir_not_ONE_cell = -1;
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 1) {
						int no_connected_bridges;
						int cell = island_in_direction(board, row, col, dir, no_connected_bridges);
						if (cell == 1) {
							no_ONE_cell++;
						}
					}
					else if (available_bridges[dir] > 1) {
						dir_not_ONE_cell = dir;
					}
				}
				if (no_ONE_cell == 3) {
					setBridgesInOneDirection(board, row, col, 1, dir_not_ONE_cell);
					TRACE(1, 22)
				}
			}
			else if ((no_remaining_bridges == 4 && no_available_bridges == 5) ||
				(no_remaining_bridges == 5 && no_available_bridges == 6) ||
				(no_remaining_bridges == 6 && no_available_bridges == 7)) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] == 2) {
						setBridgesInOneDirection(board, row, col, 1, dir);
						TRACE(1, 23)
					}
				}
			}
			else if (no_remaining_bridges == 7 && no_available_bridges == 8) {
				for (int dir = 0; dir < 4; dir++) {
					if (available_bridges[dir] > 0) {
						setBridgesInOneDirection(board, row, col, 1, dir);
						TRACE(1, 24)
					}
				}
			}
			break;
		}
		default: {
			break;
		}
		}
	}
	return true;
}

int island_in_direction(Board& board, int row, int col, int direction, int& no_connected_bridges)
{
	int row_directions[4] = { 0, -1, 0, 1 };
	int col_directions[4] = { 1, 0, -1, 0 };

	int bridge_offset[4] = { 0, -10, 0, -10 };

	int row_next = row + row_directions[direction];
	int col_next = col + col_directions[direction];
	while (board.data[row_next][col_next].value <= 0) {
		row_next += row_directions[direction];
		col_next += col_directions[direction];
	}

	no_connected_bridges = find_no_bridges_in_island(board, row_next, col_next);

	return board.data[row_next][col_next].value;
}

bool check_nodes_connected(Board& board, int row, int col, int direction, int level, bool follow_available_bridges, bool follow_existing_bridges)
{
	int row_directions[4] = { 0, -1, 0, 1 };
	int col_directions[4] = { 1, 0, -1, 0 };
	int next_direction[4] = { 1, 2, 3, 0 };

	int existing_bridges[4];
	int available_bridges[4];
	int no_existing_bridges;
	int no_available_bridges;

	int start_direction = direction;

	static bool trace = false;

	if (level == 0 && row == -1 && col == -1) {
		findFirstIsland(board, false, row, col);
		saveVisitedNodes(board);
		clearVisitedNodes(board);
	}

	if (board.data[row][col].visited) {
		return false;
	}

	find_bridges_in_island(board, row, col, existing_bridges, available_bridges, no_existing_bridges, no_available_bridges);

	if (board.data[row][col].value - no_existing_bridges > no_available_bridges) {
		// cout << " (uh uh) ";
		return false;
	}

	board.data[row][col].visited = true;

	if (trace) {
		cout << "--------------------------------------" << endl;
		printBoard(board, row, col);
	}

	for (int dir = 0; dir < 4; dir++) {

		int row_next = row;
		int col_next = col;

		bool node_available = false;
		if (follow_existing_bridges) {
			if (existing_bridges[direction] > 0) {
				node_available = true;
			}
		}
		if (!node_available && follow_available_bridges) {
			if (available_bridges[direction] > 0) {
				node_available = true;
			}
		}

		if (node_available) {
			row_next += row_directions[direction];
			col_next += col_directions[direction];
			while (board.data[row_next][col_next].value <= 0) {
				row_next += row_directions[direction];
				col_next += col_directions[direction];
			}
			int next_level = level + 1;
			check_nodes_connected(board, row_next, col_next, direction, next_level, follow_available_bridges, follow_existing_bridges);

		}
		direction = next_direction[direction];
	}

	bool allNodesReached = false;

	if (level == 0) {
		allNodesReached = areAllNodesVisited(board);

		restoreVisitedNodes(board);
	}

	return allNodesReached;
}

enum analyzis_result find_missing_bridges(Board& board, int level, int row, int col, int direction, int& iteration, int& noOfGuesses)
{
	int row_directions[4] = { 0, -1, 0, 1 };
	int col_directions[4] = { 1, 0, -1, 0 };

	int next_direction[4] = { 1, 2, 3, 0 };
	//int next_direction[4] = { 3, 0, 1, 2 };

	int opposite_direction[4] = { 2, 3, 0, 1 };

	int existing_bridges[4];
	int available_bridges[4];

	int no_existing_bridges;
	int no_available_bridges;

	static bool trace = (settings.print_info_during_guess ? true : false);

	vector<vector<Node>> last_board_1(board.nrows, vector<Node>(board.ncols));
	vector<vector<Node>> last_board_2(board.nrows, vector<Node>(board.ncols));

	static bool saved_has_solution_file;

	if (level == 0) {
		findFirstIsland(board, true, row, col);
		direction = 0;

		saved_has_solution_file = solution.has_solution_file;
		solution.has_solution_file = false;
	}

	find_bridges_in_island(board, row, col, existing_bridges, available_bridges, no_existing_bridges, no_available_bridges);

	saveBoard(board, last_board_1);

	int start_direction = direction;

	if (no_available_bridges > 0) {
		for (int dir = 0; dir < 4; dir++) {
			analyzis_result result = analyzis_result::unknown;
			for (int step = 0; step < 2; step++) {

				if (available_bridges[direction] > 0) {

					int no_existing_bridges_in_direction = existing_bridges[direction];
					int no_available_bridges_in_direction = available_bridges[direction];

					int no_bridges_to_set = 0;

					if (no_existing_bridges_in_direction == step) {
						no_bridges_to_set = 1 + step;
					}
					else {
						continue;
					}

					setBridgesInOneDirection(board, row, col, no_bridges_to_set, direction);
					if (trace) {
						cout << "-------------- after bridge(s) set ------------------------" << endl;
						printBoard(board, row, col);
					}

					int row_next = row + row_directions[direction];
					int col_next = col + col_directions[direction];

					string strBridge;
					if (direction == 0 || direction == 2) {
						strBridge = (no_bridges_to_set == 1 ? "-" : "=");
					}
					else {
						strBridge = (no_bridges_to_set == 1 ? "|" : "||");
					}
					std::stringstream ss;
					ss << "(" << row_next << ", " << col_next << "):" << strBridge << " ";

					noOfGuesses++;
					int noShortCircuitsRemoved;
					int noImpossibledBridgesRemoved;
					bool bridgesAdded = true;
					bool error_in_board = false;
					bool checkShortCircuits = (settings.remove_short_circuits == 2 || settings.remove_short_circuits == 3);
					bool checkImpossibleBridges = (settings.remove_impossible_bridges == 2 || settings.remove_impossible_bridges == 3);
					while (bridgesAdded) {
						saveBoard(board, last_board_2);

						iteration++;
						error_in_board = !analyzeBoard(board, iteration, checkShortCircuits, checkImpossibleBridges, true, noShortCircuitsRemoved, noImpossibledBridgesRemoved);

						if (error_in_board) {
							break;
						}

						if (compareBoards(board, last_board_2)) {
							bridgesAdded = false;
						}
					}

					if (error_in_board) {
						result = analyzis_result::failed;
						break;
					}

					bool allSet = areAllBridgesSet(board, false);
					bool allConnected = check_nodes_connected(board, -1, -1, 0, 0, false, true);
					bool isSolved = allSet && allConnected;

					if (isSolved) {
						if (settings.print_successful_guess && !settings.print_is_silent) {
							cout << " Correct guess at " << ss.str();
							if (level == 0) {
								cout << endl;
							}
						}
						solution.has_solution_file = saved_has_solution_file;

						return analyzis_result::solved;
					}
					else {
						if (allSet && !allConnected) {
							result = analyzis_result::failed;
							break;
						}

						while (board.data[row_next][col_next].value <= 0) {
							row_next += row_directions[direction];
							col_next += col_directions[direction];
						}

						if (trace) {
							cout << "-------------- before next recursion ------------------------" << endl;
							printBoard(board, row, col);
						}

						int next_level = level + 1;
						result = find_missing_bridges(board, next_level, row_next, col_next, direction, iteration, noOfGuesses);
						if (result == analyzis_result::solved) {
							if (settings.print_successful_guess && !settings.print_is_silent) {
								cout << ss.str();
								if (level == 0) {
									cout << " (level 1 is leftmost)" << endl;
								}
							}
							return analyzis_result::solved;;
						}
						else if (result == analyzis_result::unknown) {
							find_bridges_in_island(board, row, col, existing_bridges, available_bridges, no_existing_bridges, no_available_bridges);
						}
					}
				}
			}
			if (result == analyzis_result::failed) {
				restoreBoard(board, last_board_1);
				if (trace) {
					cout << "-------------- after restore board ------------------------" << endl;
					printBoard(board, row, col);
				}
				find_bridges_in_island(board, row, col, existing_bridges, available_bridges, no_existing_bridges, no_available_bridges);
			}
			direction = next_direction[direction];
			if (direction == opposite_direction[start_direction]) {
				continue;
			}
		}
	}
	if (no_available_bridges == 0) {

		int row_next;
		int col_next;
		findFirstIsland(board, true, row_next, col_next);
		analyzis_result result = find_missing_bridges(board, level + 1, row_next, col_next, direction, iteration, noOfGuesses);
		return result;
	}

	return analyzis_result::failed;
}

void removeBridgesCausingShortCircuits(Board& board, int row, int col, int available_bridges[4], int& noInvalidBridgesRemoved)
{
	int row_directions[4] = { 0, -1, 0, 1 };
	int col_directions[4] = { 1, 0, -1, 0 };

	int one_bridge[4] = { ONE_HORIZONTAL, ONE_VERTICAL, ONE_HORIZONTAL, ONE_VERTICAL };
	int two_bridges[4] = { TWO_HORIZONTAL, TWO_VERTICAL, TWO_HORIZONTAL, TWO_VERTICAL };

	int row_next = row;
	int col_next = col;

	noInvalidBridgesRemoved = 0;

	for (int dir = 0; dir < 4; dir++) {
		int next_row = row;
		int next_col = col;

		int no_available_bridges = available_bridges[dir];

		if (no_available_bridges > 0) {

			int number_of_bridges = 1;

			int restore_bridge = 0;

			next_row += row_directions[dir];
			next_col += col_directions[dir];

			int cell_value = board.data[next_row][next_col].value;

			while (cell_value <= 0) {
				if (cell_value == one_bridge[dir] && no_available_bridges == 1) {
					restore_bridge = one_bridge[dir];
					board.data[next_row][next_col].value = two_bridges[dir];
				}
				else if (cell_value == 0 && no_available_bridges == 2) {
					restore_bridge = 0;
					board.data[next_row][next_col].value = two_bridges[dir];
				}
				else if (cell_value == 0 && no_available_bridges == 1) {
					restore_bridge = 0;
					board.data[next_row][next_col].value = one_bridge[dir];
				}

				next_row += row_directions[dir];
				next_col += col_directions[dir];

				cell_value = board.data[next_row][next_col].value;
			}

			int level = 0;
			bool allNodesReached = check_nodes_connected(board, -1, -1, dir, level, true, true);

			int next_row = row;
			int next_col = col;

			next_row += row_directions[dir];
			next_col += col_directions[dir];

			while (board.data[next_row][next_col].value < 0) {
				board.data[next_row][next_col].value = restore_bridge;
				next_row += row_directions[dir];
				next_col += col_directions[dir];
			}

			if (!allNodesReached) {
				available_bridges[dir]--;
				noInvalidBridgesRemoved++;
			}


		}
	}

}

int is_island_available_in_direction(Board& board, int row, int col, int direction, int& no_available_bridges_in_cell, int& row_found, int& col_found)
{
	int row_directions[4] = { 0, -1, 0, 1 };
	int col_directions[4] = { 1, 0, -1, 0 };

	int one_bridge[4] = { ONE_HORIZONTAL, ONE_VERTICAL, ONE_HORIZONTAL, ONE_VERTICAL };

	int available_island = 0;
	no_available_bridges_in_cell = 0;

	int row_next = row + row_directions[direction];
	int col_next = col + col_directions[direction];
	int cell_value = board.data[row_next][col_next].value;

	while (cell_value == 0 || cell_value == one_bridge[direction]) {

		row_next += row_directions[direction];
		col_next += col_directions[direction];

		cell_value = board.data[row_next][col_next].value;
	}

	if (cell_value > 0) {
		row_found = row_next;
		col_found = col_next;

		if (!board.data[row_next][col_next].allBridgesSet) {

			int existing_bridges = 0;

			if (board.data[row_next][col_next + 1].value == ONE_HORIZONTAL) {
				existing_bridges++;
			}
			else if (board.data[row_next][col_next + 1].value == TWO_HORIZONTAL) {
				existing_bridges += 2;
			}

			if (board.data[row_next - 1][col_next].value == ONE_VERTICAL) {
				existing_bridges++;
			}
			else if (board.data[row_next - 1][col_next].value == TWO_VERTICAL) {
				existing_bridges += 2;
			}

			if (board.data[row_next][col_next - 1].value == ONE_HORIZONTAL) {
				existing_bridges++;
			}
			else if (board.data[row_next][col_next - 1].value == TWO_HORIZONTAL) {
				existing_bridges += 2;
			}

			if (board.data[row_next + 1][col_next].value == ONE_VERTICAL) {
				existing_bridges++;
			}
			else if (board.data[row_next + 1][col_next].value == TWO_VERTICAL) {
				existing_bridges += 2;
			}

			if (existing_bridges < cell_value) {
				available_island = board.data[row_next][col_next].value;
				no_available_bridges_in_cell = cell_value - existing_bridges;
			}
		}

	}

	return available_island;
}

void find_bridges_in_island(Board& board, int row, int col, int existing_bridges[4], int available_bridges[4], int& no_existing_bridges, int& no_available_bridges)
{
	// right, up, left down
	int row_directions[4] = { 0, -1, 0, 1 };
	int col_directions[4] = { 1, 0, -1, 0 };

	int no_available_bridges_in_cell;
	int row_found;
	int col_found;

	no_existing_bridges = 0;
	no_available_bridges = 0;

	bool check_available_bridges = !board.data[row][col].allBridgesSet;

	for (int dir = 0; dir < 4; dir++) {

		int row_next = row + row_directions[dir];
		int col_next = col + col_directions[dir];

		int cell = board.data[row_next][col_next].value;

		existing_bridges[dir] = 0;
		available_bridges[dir] = 0;

		if ((dir == 0 || dir == 2) && cell == ONE_HORIZONTAL) {
			existing_bridges[dir] = 1;
			no_existing_bridges++;

			if (check_available_bridges && is_island_available_in_direction(board, row_next, col_next, dir, no_available_bridges_in_cell, row_found, col_found) > 0) {
				available_bridges[dir] = 1;
				no_available_bridges++;
			}
		}
		else if ((dir == 0 || dir == 2) && cell == TWO_HORIZONTAL) {
			existing_bridges[dir] = 2;
			no_existing_bridges += 2;

		}
		else if ((dir == 1 || dir == 3) && cell == ONE_VERTICAL) {
			existing_bridges[dir] = 1;
			no_existing_bridges++;

			if (check_available_bridges && is_island_available_in_direction(board, row_next, col_next, dir, no_available_bridges_in_cell, row_found, col_found) > 0) {
				available_bridges[dir] = 1;
				no_available_bridges++;
			}
		}
		else if ((dir == 1 || dir == 3) && cell == TWO_VERTICAL) {
			existing_bridges[dir] = 2;
			no_existing_bridges += 2;
		}
		else if (check_available_bridges && cell == 0) {
			int available_island = is_island_available_in_direction(board, row_next, col_next, dir, no_available_bridges_in_cell, row_found, col_found);

			if (available_island > 0) {
				if (no_available_bridges_in_cell == 1) {
					available_bridges[dir] = 1;
					no_available_bridges += 1;
				}
				else if (no_available_bridges_in_cell >= 2) {
					available_bridges[dir] = 2;
					no_available_bridges += 2;
				}
			}

		}

	}

	if (check_available_bridges) {
		int no_required_bridges = board.data[row][col].value;
		int no_remaining_bridges = no_required_bridges - no_existing_bridges;

		if (no_remaining_bridges < 2) {
			no_available_bridges = 0;
			for (int dir = 0; dir < 4; dir++) {
				if (available_bridges[dir] > 0) {
					available_bridges[dir] = no_remaining_bridges;
					no_available_bridges += no_remaining_bridges;
				}
			}
		}
	}

}

int find_no_bridges_in_island(Board& board, int row, int col)
{
	// right, up, left down
	int row_directions[4] = { 0, -1, 0, 1 };
	int col_directions[4] = { 1, 0, -1, 0 };

	int bridge_offset[4] = { 0, -10, 0, -10 };

	int no_connected_bridges = 0;

	for (int dir = 0; dir < 4; dir++) {

		int row_next = row + row_directions[dir];
		int col_next = col + col_directions[dir];

		if (board.data[row_next][col_next].value == bridge_offset[dir] - 1) {
			no_connected_bridges++;
		}
		else if (board.data[row_next][col_next].value == bridge_offset[dir] - 2) {
			no_connected_bridges += 2;
		}

	}

	return no_connected_bridges;

}

void findFirstIsland(Board& board, bool require_not_filled, int& row, int& col)
{
	row = 1;
	col = 1;
	while (board.data[row][col].value <= 0 || (board.data[row][col].allBridgesSet && require_not_filled)) {
		if (col == board.ncols - 1) {
			row++;
			col = 1;
		}
		else {
			col++;
		}
	}
}

void setBridgesInOneDirection(Board& board, int row, int col, int number_of_bridges, int direction)
{
	int row_directions[4] = { 0, -1, 0, 1 };
	int col_directions[4] = { 1, 0, -1, 0 };

	int bridge_offset[4] = { 0, -10, 0, -10 };

	int bridges = bridge_offset[direction] - number_of_bridges;
	int one_bridge = bridge_offset[direction] - 1;

	int row_inc = row_directions[direction];
	int col_inc = col_directions[direction];

	int row_next = row + row_inc;
	int col_next = col + col_inc;
	while (board.data[row_next][col_next].value == 0 || board.data[row_next][col_next].value == one_bridge) {
		board.data[row_next][col_next].value = bridges;
		CHECK_WITH_SOLUTION
			row_next += row_inc;
		col_next += col_inc;
	}
}

void restoreBridgesInOneDirection(Board& board, int row, int col, int restore, int direction)
{
	int row_directions[4] = { 0, -1, 0, 1 };
	int col_directions[4] = { 1, 0, -1, 0 };

	int row_next = row + row_directions[direction];
	int col_next = col + col_directions[direction];
	while (board.data[row_next][col_next].value < 0) {
		board.data[row_next][col_next].value = restore;
		row_next += row_directions[direction];
		col_next += col_directions[direction];
	}
}

void initBoard(vector<vector<int>> puzzle, Board& board)
{
	int nrows = (int)(puzzle.size()) + 2;
	int ncols = (int)(puzzle[0].size()) + 2;


	// first row
	for (int col = 0; col < ncols; col++) {
		board.data[0][col].value = TWO_HORIZONTAL;
	}

	// Real rows
	for (int row = 1; row < nrows - 1; row++)
	{
		// first column
		board.data[row][0].value = TWO_VERTICAL;

		for (int col = 1; col < ncols - 1; col++)
		{
			board.data[row][col].value = puzzle[row - 1][col - 1];
		}

		// last column
		board.data[row][ncols - 1].value = TWO_VERTICAL;
	}

	// last row
	for (int col = 0; col < ncols; col++) {
		board.data[nrows - 1][col].value = TWO_HORIZONTAL;
	}
}

void setSolution(vector<vector<int>> puzzle_solution)
{
	int nrows = (int)(puzzle_solution.size());
	int ncols = (int)(puzzle_solution[0].size());

	solution.solution.resize(nrows);
	for (int row = 0; row < nrows; row++)
	{
		solution.solution[row].resize(ncols);
		for (int col = 0; col < ncols; col++)
		{
			solution.solution[row][col] = puzzle_solution[row][col];
		}
	}

	solution.has_solution_file = true;
}

void checkWithSolution(Board& board, int row, int col)
{
	if (solution.has_solution_file == false) {
		return;
	}

	int current = board.data[row][col].value;

	int expected = solution.solution[row - 1][col - 1];

	if (current == 0) {
		;// ok
	}
	else if (current == ONE_HORIZONTAL && expected == TWO_HORIZONTAL) {
		;// ok
	}
	else if (current == ONE_VERTICAL && expected == TWO_VERTICAL) {
		;// ok
	}
	else if (current != expected) {
		cout << "error: Expected: " << expected << " but found: " << current << " at row: " << row << ", col: " << col << '\a' << endl;
		if (settings.print_board_when_solution_mismatch) {
			printBoard(board);
			printSolution();
		}
	}
}

void printBoard(Board& board, int row_show, int col_show)
{
	for (int row = 1; row < board.nrows - 1; row++)
	{
		for (int col = 1; col < board.ncols - 1; col++)
		{
			int cell = board.data[row][col].value;
			if (cell == 0) {
				cout << "   ";
			}
			else if (cell > 0) {
				if (settings.print_board_with_colors) {
					if (row == row_show && col == col_show) {
						cout << " " << "\x1B[34m" << cell << "\033[0m" << " ";
					}
					else {
						int no_bridges_in_cell = find_no_bridges_in_island(board, row, col);
						if (no_bridges_in_cell == cell) {
							cout << " " << "\x1B[32m" << cell << "\033[0m" << " ";
						}
						else {
							cout << " " << "\x1B[31m" << cell << "\033[0m" << " ";
						}
					}
				}
				else {
					cout << " " << cell << " ";
				}
			}
			else if (cell == ONE_HORIZONTAL) {
				cout << " - ";
			}
			else if (cell == TWO_HORIZONTAL) {
				cout << " = ";
			}
			else if (cell == ONE_VERTICAL) {
				cout << " | ";
			}
			else if (cell == TWO_VERTICAL) {
				cout << " ||";
			}
			else {
				cout << " * ";
			}
		}
		cout << endl;
	}
}

void printSolution()
{
	if (solution.has_solution_file) {
		cout << " ------- Solution: -------" << endl;

		int nrows = (int)(solution.solution.size());
		int ncols = (int)(solution.solution[0].size());

		for (int row = 0; row < nrows; row++)
		{
			for (int col = 0; col < ncols; col++)
			{
				int cell = solution.solution[row][col];
				if (cell == 0) {
					cout << "   ";
				}
				else if (cell > 0) {
					cout << " " << cell << " ";
				}
				else if (cell == ONE_HORIZONTAL) {
					cout << " - ";
				}
				else if (cell == TWO_HORIZONTAL) {
					cout << " = ";
				}
				else if (cell == ONE_VERTICAL) {
					cout << " | ";
				}
				else if (cell == TWO_VERTICAL) {
					cout << " ||";
				}
				else {
					cout << " * ";
				}
			}
			cout << endl;
		}
	}
}

void printPuzzle(vector<vector<int>>& puzzle)
{
	int nrows = (int)(puzzle.size());
	int ncols = (int)(puzzle[0].size());

	for (int row = 0; row < nrows; row++)
	{
		for (int col = 0; col < ncols; col++)
		{
			int cell = puzzle[row][col];
			if (cell == 0) {
				cout << "   ";
			}
			else {
				cout << " " << cell << " ";
			}
		}
		cout << endl;
	}
}

void saveBoard(Board& board, vector<vector<Node>>& saved_board)
{
	for (int row = 1; row < board.nrows - 1; row++)
	{
		for (int col = 1; col < board.ncols - 1; col++)
		{
			saved_board[row][col] = board.data[row][col];
		}
	}
}

bool compareBoards(Board& board, vector<vector<Node>> saved_board)
{
	for (int row = 1; row < board.nrows - 1; row++)
	{
		for (int col = 1; col < board.ncols - 1; col++)
		{
			if (saved_board[row][col].value != board.data[row][col].value) {
				return false;
			}
		}
	}
	return true;
}

void restoreBoard(Board& board, vector<vector<Node>> saved_board)
{
	for (int row = 1; row < board.nrows - 1; row++)
	{
		for (int col = 1; col < board.ncols - 1; col++)
		{
			board.data[row][col] = saved_board[row][col];
		}
	}
}

int find_total_no_bridges_set(Board& board)
{
	int noBridges = 0;

	int currentBridge = 0;

	for (int row = 1; row < board.nrows - 1; row++)
	{
		for (int col = 1; col < board.ncols - 1; col++)
		{
			if (currentBridge > 0) {
				if (board.data[row][col].value > 0) {
					noBridges += currentBridge;
					currentBridge = 0;
				}
			}
			else if (currentBridge == 0) {
				if (board.data[row][col].value == ONE_HORIZONTAL) {
					currentBridge = 1;
				}
				else if (board.data[row][col].value == TWO_HORIZONTAL) {
					currentBridge = 2;
				}
			}
		}
	}

	currentBridge = 0;

	for (int col = 1; col < board.ncols - 1; col++)
	{
		for (int row = 1; row < board.nrows - 1; row++)
		{
			if (currentBridge > 0) {
				if (board.data[row][col].value > 0) {
					noBridges += currentBridge;
					currentBridge = 0;
				}
			}
			else if (currentBridge == 0) {
				if (board.data[row][col].value == ONE_VERTICAL) {
					currentBridge = 1;
				}
				else if (board.data[row][col].value == TWO_VERTICAL) {
					currentBridge = 2;
				}
			}
		}
	}

	return noBridges;
}

bool areAllBridgesSet(Board& board, bool logErrors)
{
	for (int row = 1; row < board.nrows - 1; row++)
	{
		for (int col = 1; col < board.ncols - 1; col++)
		{
			if (board.data[row][col].isIsland()) {
				if (board.data[row][col].allBridgesSet == false) {
					return false;
				}
			}
		}
	}
	return true;
}

int remainingBridges(Board& board, bool logErrors)
{
	int remaining_bridges = 0;

	for (int row = 1; row < board.nrows - 1; row++)
	{
		for (int col = 1; col < board.ncols - 1; col++)
		{
			if (board.data[row][col].isIsland()) {
				if (board.data[row][col].allBridgesSet == false) {
					int no_bridges_in_cell = find_no_bridges_in_island(board, row, col);
					if (no_bridges_in_cell != board.data[row][col].value) {
						remaining_bridges += (board.data[row][col].value - no_bridges_in_cell);
					}
				}
			}
		}
	}
	return remaining_bridges / 2;
}

void swap(int& a, int& b)
{
	int temp = a;
	a = b;
	b = temp;
}

void rotatePuzzle90degree(vector<vector<int>>& puzzle)
{
	int nrows = (int)(puzzle.size());
	int ncols = (int)(puzzle[0].size());

	int rotated_rows = ncols;
	int rotated_cols = nrows;

	vector<vector<int>> rotated_puzzle(rotated_rows, vector<int>(rotated_cols));

	for (int i = 0; i < rotated_rows; i++) {
		for (int j = 0; j < rotated_cols; j++) {
			rotated_puzzle[i][j] = puzzle[nrows - j - 1][i];
		}
	}

	puzzle.clear();

	puzzle.resize(rotated_rows);
	for (int i = 0; i < rotated_rows; i++) {
		puzzle[i].resize(rotated_cols);
		for (int j = 0; j < rotated_cols; j++) {
			puzzle[i][j] = rotated_puzzle[i][j];
		}
	}

	// Reversing bridge directions
	for (int i = 0; i < rotated_rows; i++) {
		for (int j = 0; j < rotated_cols; j++) {
			if (puzzle[i][j] == ONE_HORIZONTAL) {
				puzzle[i][j] = ONE_VERTICAL;
			}
			else if (puzzle[i][j] == TWO_HORIZONTAL) {
				puzzle[i][j] = TWO_VERTICAL;
			}
			else if (puzzle[i][j] == ONE_VERTICAL) {
				puzzle[i][j] = ONE_HORIZONTAL;
			}
			else if (puzzle[i][j] == TWO_VERTICAL) {
				puzzle[i][j] = TWO_HORIZONTAL;
			}
		}
	}

}

void reverseRowsInPuzzle(vector<vector<int>>& puzzle)
{
	int nrows = (int)(puzzle.size());
	int ncols = (int)(puzzle[0].size());

	for (int i = 0; i < nrows; i++) {
		for (int j = 0; j < ncols / 2; j++) {
			swap(puzzle[i][j], puzzle[i][ncols - j - 1]);
		}
	}
}

void clearVisitedNodes(Board& board) {
	for (int row = 1; row < board.nrows - 1; row++)
	{
		for (int col = 1; col < board.ncols - 1; col++)
		{
			board.data[row][col].visited = false;
		}
	}
}

bool areAllNodesVisited(Board& board) {
	for (int row = 1; row < board.nrows - 1; row++)
	{
		for (int col = 1; col < board.ncols - 1; col++)
		{
			if (board.data[row][col].isIsland()) {
				if (board.data[row][col].visited == false) {
					return false;
				}
			}
		}
	}
	return true;
}

void saveVisitedNodes(Board& board)
{
	for (int row = 1; row < board.nrows - 1; row++)
	{
		for (int col = 1; col < board.ncols - 1; col++)
		{
			if (board.data[row][col].isIsland()) {
				board.data[row][col].visited_saved = board.data[row][col].visited;
			}
		}
	}
}

void restoreVisitedNodes(Board& board)
{
	for (int row = 1; row < board.nrows - 1; row++)
	{
		for (int col = 1; col < board.ncols - 1; col++)
		{
			if (board.data[row][col].isIsland()) {
				board.data[row][col].visited = board.data[row][col].visited_saved;
			}
		}
	}
}
