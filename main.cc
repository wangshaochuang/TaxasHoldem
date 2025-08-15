#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include <ctime>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

std::string human_poker(int p) {
    std::string colors[4] = {"Spades", "Hearts", "Clubs", "Diamonds"};
    std::vector<char> numbers = {'2','3','4','5','6','7','8','9','T','J','Q','K','A'};
    return colors[p/13]+numbers[p%13];
}

void show_human_cards(std::vector<int> &vs) {
    for (auto v : vs) {
        std::cout << human_poker(v) << " ";
    }
    std::cout<<std::endl;
}

class Poker {
private:
    std::vector<int> pokers;
    int index = 0;
    std::mt19937 rng;
    void init_rng() {
        unsigned seed = static_cast<unsigned>(std::time(nullptr));
        rng = std::mt19937(seed);
    }
public:
    Poker(int c1, int c2) {
        pokers.clear();
        for (int i = 0; i < 52; i++) {
            if (i != c1 || i != c2) {
                pokers.push_back(i);
            }
        }
        init_rng();
    }
    Poker(std::vector<int> &p) {
        pokers = p;
        init_rng();
    }
    Poker() {
        for (int i = 0; i < 52; i++) {
            pokers.push_back(i);
        }
        init_rng();
    };
    void shuffle() {
        index = 0;
        std::shuffle(pokers.begin(), pokers.end(), rng);
    };
    int get_one(int &value) {
        if (index < 52) {
            value = pokers[index++];
            return 0;
        } else
            return -1;
    }

    void get_all(std::vector<int> &values) {
        values = pokers;
    }

    void show_all() {
        show_human_cards(pokers);
    }
};

class Table {
private:
    std::vector<int> pokers = {-1, -1, -1, -1, -1};
    int stage = 0;
public:
    Table() {};
    void new_game() {
        for (int i = 0; i < 5; i++)
            pokers[i] = -1;
        stage = 0;
    }

    int next_stage(Poker &poker) {
        int card;
        if (stage == 0) {
            for (int i = 0; i < 3; i++) {
                poker.get_one(card);
                pokers[i] = card;
            }
        } else if (stage == 1) {
            poker.get_one(card);
            poker.get_one(card);
            pokers[3] = card;
        } else if (stage == 2) {
            poker.get_one(card);
            poker.get_one(card);
            pokers[4] = card;
        } else {
            std::cout << "error stage" << std::endl;
            return -1;
        }
        stage++;
        return 0;
    }

    int get_common_cards(std::vector<int> &cards) {
        cards = pokers;
        return stage;
    }

    void show() {
        show_human_cards(pokers);
    }
};

class Player {
private:
    uint64_t key = 0;
    int money;
    int card1, card2;
    int status = 0;
public:
    std::string name;
    Player() {}
    Player(std::string name, int money) {
        this->name = name;
        this->money = money;
    }
    uint64_t get_key() {return key; }

    void have_cards(int a, int b) {
        card1 = a;
        card2 = b;
        status = 1;
    }

    int get_cards(int &a, int &b) {
        a = card1;
        b = card2;
        return status;
    }

    bool can_call(int m) {
        if (m <= money && status == 1)
            return true;
        return false;
    }

    int call(int m) {
        if (can_call(m)) {
            money -= m;
            return 0;
        }
        return -1;
    }

    void fold() {
        status = 0;
    }
};

class Results {
public:
    int score = -1;
    std::vector<int> cards;

    void show() {
        std::cout << "score: " << std::hex << score << " with cards: ";
        show_human_cards(cards);
    }
};

class Score {
private:
    Results scores;
    std::vector<int> cards = {-1, -1, -1, -1, -1, -1,-1};

    int to_score(int level, std::vector<int> &value) {
        int score = 0;
        int place = 20;
        score += (level << place);
        place -= 4;
        for (auto v : value) {
            score += (v << place);
            place -= 4;
        }
        return score;
    }

    int score_compute(std::vector<int> &cs) {
        int max_score = 0xffffff;
        std::vector<int> colors = cs;
        std::vector<int> numbers = cs;

        bool is_flush = true;
        bool is_4kind = false;
        bool is_3kind = false;
        bool is_full_house = false;
        bool is_straight = true;
        int pairs_number = 0;
        std::vector<int> pairs;
        std::vector<int> values;

        int flush_color = colors[0] = cs[0] / 13;
        numbers[0] = cs[0] % 13;
        for (int i = 1; i < 5; i++) {
            colors[i] = cs[i] / 13;
            numbers[i] = cs[i] % 13;

            if (is_flush && colors[i] != flush_color) {
                is_flush = false;
            }
        }

        std::vector<int> sort_numbers = numbers;
        sort(sort_numbers.begin(), sort_numbers.end(), std::greater<int>());
        for (int i = 0; i < 4; i++) {
            if (sort_numbers[i] - 1 != sort_numbers[i+1]) {
                is_straight = false;
            }
            if (sort_numbers[i] == sort_numbers[i+1]) {
                pairs_number++;
                pairs.push_back(sort_numbers[i]);
            }
        }
        if (sort_numbers[0] == 12 && sort_numbers[1] == 0 && sort_numbers[2] == 1 && sort_numbers[3] == 2 && sort_numbers[4] == 3)
            is_straight = true;
        if (sort_numbers[0] == sort_numbers[3]) {
            is_4kind = true;
            values.push_back(sort_numbers[0]);
            values.push_back(sort_numbers[4]);
        } else if (sort_numbers[1] == sort_numbers[4]) {
            is_4kind = true;
            values.push_back(sort_numbers[1]);
            values.push_back(sort_numbers[0]);
        }

        if (is_flush && is_straight) {
            return to_score(0xf, sort_numbers);
        }
        if (is_4kind) {
            return to_score(0xd, values);
        }

        if (sort_numbers[1] == sort_numbers[3]) {
            is_3kind = true;
            values.push_back(sort_numbers[1]);
            values.push_back(sort_numbers[0]);
            values.push_back(sort_numbers[4]);
        }
        if (sort_numbers[0] == sort_numbers[2]) {
            if (sort_numbers[3] == sort_numbers[4]) {
                is_full_house = true;
                values.push_back(sort_numbers[0]);
                values.push_back(sort_numbers[3]);
            }
            else {
                is_3kind = true;
                values.push_back(sort_numbers[0]);
                values.push_back(sort_numbers[3]);
                values.push_back(sort_numbers[4]);
            }
        } else if (sort_numbers[2] == sort_numbers[4]) {
            if (sort_numbers[0] == sort_numbers[1]) {
                is_full_house = true;
                values.push_back(sort_numbers[2]);
                values.push_back(sort_numbers[1]);
            }
            else {
                is_3kind = true;
                values.push_back(sort_numbers[2]);
                values.push_back(sort_numbers[0]);
                values.push_back(sort_numbers[1]);
            }
        }
        if (is_full_house) {
            return to_score(0xc, values);
        }

        if (is_flush) {
            return to_score(0xb, sort_numbers);
        }

        if (is_straight) {
            return to_score(0xa, sort_numbers);
        }

        if (is_3kind) {
            return to_score(0x9, values);
        }

        if (pairs_number == 2) {
            values=pairs;
            bool is_find = false;
            for (int i = 0; i < 5; i++) {
                is_find = false;
                for (auto p : pairs) {
                    if (p == sort_numbers[i]) {
                        is_find = true;
                    }
                }
                if (!is_find)
                    values.push_back(sort_numbers[i]);
            }
            return to_score(0x8, values);
        }

        if (pairs_number == 1) {
            values=pairs;
            bool is_find = false;
            for (int i = 0; i < 5; i++) {
                is_find = false;
                for (auto p : pairs) {
                    if (p == sort_numbers[i]) {
                        is_find = true;
                    }
                }
                if (!is_find)
                    values.push_back(sort_numbers[i]);
            }
            return to_score(0x7, values);
        }

        return to_score(0x6, sort_numbers);
    }

    int cards_score() {
        std::vector<int> cards_compute;
        for (auto c : cards) {
            if (c != -1) cards_compute.push_back(c);
        }
        return score_compute(cards_compute);
    }

    Results compute() {
        Results r;
        int max_score = 0;
        int ex1 = 0;
        int ex2 = 1;
    
        for (int i = 0; i < 7; i++) {
            for (int j = i + 1; j < 7; j++) {
                int r1 = cards[i];
                int r2 = cards[j];
                cards[i] = cards[j] = -1;
                int new_score = cards_score();
                if (new_score > max_score) {
                    ex1 = i;
                    ex2 = j;
                    max_score = new_score;
                }
                cards[i] = r1;
                cards[j] = r2;
            }
        }
        r.score = max_score;
        for (int i = 0; i < cards.size(); i++) {
            if (i != ex1 && i != ex2)
                r.cards.push_back(cards[i]);
        }

        return r;
    }
public:
    Score(std::vector<int> &values) {
        cards = values;
    }

    Score(Table t, Player p) {
        int a,b;
        t.get_common_cards(cards);
        p.get_cards(a, b);
        cards.push_back(a);
        cards.push_back(b);
    }

    bool do_compute() {
        bool is_valid = true;
        for (int i = 0; i < 7; i++) {
            if (cards[i] == -1) {
                is_valid = false;
                break;
            }
        }
        if (is_valid == true) {
            scores = compute();
        }
        return is_valid;
    }

    Results get_scores() {
        return scores;
    }

};

class Winner {
public:
    Results r;
    Player *p;

    Winner(Results &r, Player *p) : r(r), p(p) {};
};

void test_scores() {
    //test
    std::vector<std::vector<int>> test_cards;
    test_cards.push_back({0,1,2,3,4,5,6});
    test_cards.push_back({0,1,2,3,4,5,7});
    test_cards.push_back({12,11,10,9,8,7,6});
    test_cards.push_back({0,13,26,39,4,5,7});
    test_cards.push_back({0,13,26,14,1,5,7});
    test_cards.push_back({0,1,2,4,5,6,7});
    test_cards.push_back({13,1,2,4,16,3,17});
    test_cards.push_back({0,13,26,3,4,15,7});
    test_cards.push_back({0,13,2,14,33,15,7});
    test_cards.push_back({0,13,27,28,44,45,46});
    test_cards.push_back({0,1,2,17,18,19,34});

    for (auto tt : test_cards) {
        Score ss(tt);
        ss.do_compute();
        auto rv = ss.get_scores();
        rv.show();
    }
}

struct ParamsConfig {
    int num_players = 8;
    int loops = 1000;
    int test_card1 = 11;
    int test_card2 = 12;
};

void usage() {
    std::cout << "Usages:" << std::endl;
    std::cout << "\t-p <player number> [2~8]" << std::endl;
    std::cout << "\t-l <game loop number> [1~1000000]" << std::endl;
    std::cout << "\t-a <God player have this card> [0~51]" << std::endl;
    std::cout << "\t-b <God player have this card> [0~51]" << std::endl;
    std::cout << "\t-h show this" << std::endl;
}

int main(int argc, char **argv) {
    ParamsConfig params_config;
    int o;
    const char *optstring = "p:l:a:b:h";
    while ((o = getopt(argc, argv, optstring)) != -1) {
        switch (o) {
            case 'p': {
                int players = atoi(optarg);
                if (players < 2 || players > 8) {
                    std::cout << "error players number " << optarg << " should between [2,8]" << std::endl;
                    return -1;
                }
                params_config.num_players = players;
                break;
            }
            case 'l': {
                int loop = atoi(optarg);
                if (loop < 1 || loop > 1000000) {
                    std::cout << "error loop number " << optarg << " should between [1,1000000]" << std::endl;
                    return -1;
                }
                params_config.loops = loop;
                break;
            }
            case 'a': {
                int c = atoi(optarg);
                if (c < 0 || c > 51) {
                    std::cout << "error cards " << optarg << " should between [0,51]" << std::endl;
                    return -1;
                }
                params_config.test_card1 = c;
                break;
            }
            case 'b': {
                int c = atoi(optarg);
                if (c < 0 || c > 51) {
                    std::cout << "error cards " << optarg << " should between [0,51]" << std::endl;
                    return -1;
                }
                params_config.test_card2 = c;
                break;
            }
            case 'h': {
                usage();
                return 0;
            }
        }
    }

    Table table;
    Poker poker(params_config.test_card1, params_config.test_card2);
    std::vector<Player> players;
    for (int i = 0; i < params_config.num_players; i++) {
        players.push_back({"player"+std::to_string(i), 100});
    }

    int wins = 0;
    int both_wins = 0;
    for (int idx = 0; idx < params_config.loops; idx++) {
        table.new_game();
        poker.shuffle();

        players[0].have_cards(params_config.test_card1, params_config.test_card2);
        for (int i = 1; i < params_config.num_players; i++) {
            int c1, c2;
            poker.get_one(c1);
            poker.get_one(c2);
            players[i].have_cards(c1, c2);
        }
        table.next_stage(poker);
        table.next_stage(poker);
        table.next_stage(poker);

        std::vector<Winner> winners;

        for (int i = 0; i < params_config.num_players; i++) {
            Score s(table, players[i]);
            Results r;

            if (s.do_compute()) {
                r = s.get_scores();
            } else {
                std::cout << "should not be here" << std::endl;
                break;
            }
            if (winners.size() == 0 || winners[0].r.score == r.score) {
                winners.push_back({r, &players[i]});
            } else if (winners[0].r.score < r.score) {
                winners.clear();
                winners.push_back({r, &players[i]});
            }
        }
        for (auto w : winners) {
            if (w.p == &players[0]) {
                wins++;
                if (winners.size() > 1)
                    both_wins++;
            }
        }
#if 0
        std::cout << "Game: " << std::dec << idx << std::endl;
        table.show();
        for (auto w : winners) {
            std::cout << "players: " << w.p->name << " win the game" << std::endl;
            w.r.show();
        }
#endif 
    }
    std::cout << std::dec << "win: " << wins << "/" << params_config.loops << " both wins: " << both_wins << "/" << wins << std::endl;

#if 0
    test_scores();
#endif
    return 0;
}
