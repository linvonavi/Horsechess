#pragma once

#include "Movegen.h"

struct SearchConfig {
	int Movetime = 5000;
	int Depth;
	int StartTime;
	int VisitedNodes = 0;
	MoveList Stack;
};

inline char Position::move_eval(const Move& move) const {
	char res = 0, res1 = 0;
	res += bool(byType[PAWN] & square_bb(move.to));
	res += 2 * bool(byType[KNIGHT] & square_bb(move.to));
	res += 3 * bool(byType[BISHOP] & square_bb(move.to));
	res += 4 * bool(byType[ROOK] & square_bb(move.to));
	res += 5 * bool(byType[QUEEN] & square_bb(move.to));
	res1 += bool(byType[PAWN] & square_bb(move.from));
	res1 += 2 * bool(byType[KNIGHT] & square_bb(move.from));
	res1 += 3 * bool(byType[BISHOP] & square_bb(move.from));
	res1 += 4 * bool(byType[ROOK] & square_bb(move.from));
	res1 += 5 * bool(byType[QUEEN] & square_bb(move.from));
	res += move.info == PawnPromotionId;
	return res * 6 + res1;
}

void MoveList::sort(const Position& pos) {
	for (int i = 0; i < 36; i++) {
		temp.PriorityMoves[i].size = 0;
	}
	for (int i = 0; i < size; i++) {
		char eval = pos.move_eval(moves[i]);
		temp.PriorityMoves[eval].add(moves[i]);
	}
	int i = 0;
	for (int j = 35; j >= 0; j--) {
		for (int k = 0; k < temp.PriorityMoves[j].size; k++) {
			moves[i++] = temp.PriorityMoves[j].moves[k];
		}
	}
}



pair<Move, short> search(Position pos, SearchConfig& config, int depth = 0, short alpha = -10000, short beta = 10000) {
	config.VisitedNodes++;

	if (clock() - config.StartTime > config.Movetime) return make_pair(Move(), pos.eval());
	if (depth >= config.Depth) {
		if (depth >= config.Depth + 5) return make_pair(Move(), pos.eval());
		short best = pos.eval();
		MoveList moves;
		pos.legal_moves(moves);
		Position pos1;
		for (int i = 0; i < moves.size; i++) {
			if (!pos.board[moves.moves[i].to]) continue;
			memcpy(&pos1, &pos, sizeof(Position));
			pos1.make_move(moves.moves[i]);
			auto [next_move, score] = search(pos1, config, depth + 1, alpha, beta);
			best = pos.sideToMove == WHITE ? max(best, score) : min(best, score);
			if (pos.sideToMove == WHITE) {
				if (best >= beta) break;
				alpha = max(alpha, best);
			} else {
				if (best <= alpha) break;
				beta = min(beta, best);
			}
		}
		return make_pair(Move(), best);
	}
	MoveList moves;
	pos.legal_moves(moves);

	moves.sort(pos);

	short best = pos.sideToMove == WHITE ? -10000 : 10000;
	Move best_move;
	Position pos1;
	for (int i = 0; i < moves.size; i++) {
		memcpy(&pos1, &pos, sizeof(Position));
		pos1.make_move(moves.moves[i]);
		config.Stack.moves[depth] = moves.moves[i];
		auto [next_move, score] = search(pos1, config, depth + 1, alpha, beta);
		if (depth == 0) {
			if (pos.sideToMove == WHITE) {
				if (score > best) {
					best = score;
					best_move = moves.moves[i];
				}
			} else {
				if (score < best) {
					best = score;
					best_move = moves.moves[i];
				}
			}
		}
		best = pos.sideToMove == WHITE ? max(best, score) : min(best, score);
		if (pos.sideToMove == WHITE) {
			if (best >= beta) break;
			alpha = max(alpha, best);
		} else {
			if (best <= alpha) break;
			beta = min(beta, best);
		}
	}
	return make_pair(best_move, best);
}

Move start_search(Position& pos, SearchConfig config) {
	config.StartTime = clock();
	config.Depth = 1;
	Move res;
	short score;
	while (clock() - config.StartTime < config.Movetime) {
		auto r = search(pos, config);
		if (clock() - config.StartTime < config.Movetime) {
			res = r.first;
			score = r.second;
			cout << "time " << clock() - config.StartTime << " depth " << config.Depth << " score cp " << score << " pv " << move_to_string(res) << endl;
			if (score == 10000 || score == -10000) break;
		}
		config.Depth++;
	}
	cout << config.VisitedNodes << endl;
	return res;
}