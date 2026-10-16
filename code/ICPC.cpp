// ICPC Management System
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <cstdio>
#include <vector>
#include <cassert>
#include <algorithm>
#include <set>
#include <stack>

const std::string SUBMISSION[4] = {"Accepted", "Wrong_Answer", "Runtime_Error", "Time_Limit_Exceed"};
constexpr int ALL = -1;
constexpr int Accepted = 0;
constexpr int Wrong_Answer = 1;
constexpr int Runtime_Error = 2;
constexpr int Time_Limit_Exceed = 3;
bool is_freezed = false; // 是否封榜
bool is_started = false; // 是否开始
int duration_time = 0;
int problem_count = 0;

int QueryTeamCode(const std::string &team_name);
int ApplyTeamCode(const std::string &team_name);
bool Execute();
void PrintRankingList();

/**
 * The class Problem
 * @param last_submission_status_:       最后一次提交的状态（冻结期间仍然更新）
 * @param last_submission_time_:         最后一次提交的时间（冻结期间仍然更新）
 * @param status_submission_time_:       最后一次指定status提交的时间（冻结期间仍然更新）
 * @param accepted_submission_time_:     首次通过的时间（冻结期间仍然更新，但是罚时结果需要accepted_审核通过）
 * @param freezed_:                      该题是否处于freeze状态
 *                                       要求：未通过，已封榜，有提交
 * @param accepted_:                     [0] : 该题是否已经通过（冻结期间不更新）
 *                                       [1] : 该题是否在封榜期间通过（非冻结期间不更新）
 * @param unaccepted_submission_count_:  [0] : 错误提交次数（冻结期间不更新，如果已通过则不再更新）
 *                                       [1] : 封榜后**错误**提交次数（只在冻结期间更新，如果已通过则不计算）
 * @param freezed_submission_count_:     封榜后**总**提交次数（只在冻结期间更新）
 */
class Problem{
    private:
        int last_submission_status_, last_submission_time_;
        int status_submission_time_[4];
        int accepted_submission_time_;
        // 以上四个参数不会受到冻结影响，永远正常更新
        bool freezed_;
        bool accepted_[2];
        int unaccepted_submission_count_[2];
        int freezed_submission_count_;
        bool Freeze(int &freezed_count) {
            if ((!is_freezed) || accepted_[0] || freezed_) return false;
            // 拒绝冻结的条件：
            // 1. 尚未封榜
            // 2. 封榜以外的时段已经通过
            // 3. 已经冻结
            ++freezed_count; // 成功冻结
            freezed_ = true;
            accepted_[1] = false;
            unaccepted_submission_count_[1] = 0;
            freezed_submission_count_ = 0;
            return true;
        } // 冻结
        int CalculateTimePenalty(bool status_change) {
            if (!status_change) return 0; // 不是通过的瞬间，不计算
            return 20 * unaccepted_submission_count_[0] + accepted_submission_time_;
        } // 计算**未被冻结**的提交的罚时
        bool IsAccepted() {
            return (accepted_[0] | accepted_[1]);
        } // 题目是否通过
    public:
        Problem() : last_submission_status_(-1),
                    last_submission_time_(-1),
                    freezed_(false),
                    accepted_submission_time_(-1),
                    freezed_submission_count_(0) {
            accepted_[0] = accepted_[1] = false;
            status_submission_time_[Accepted] = -1;
            status_submission_time_[Wrong_Answer] = -1;
            status_submission_time_[Runtime_Error] = -1;
            status_submission_time_[Time_Limit_Exceed] = -1;
            unaccepted_submission_count_[0] = 0;
            unaccepted_submission_count_[1] = 0;
            return;
        }
        int Submit(int status_code, int submission_time, int &accepted_count, int &freezed_count) { // accepted_count和freezed_count用于与Team对接，维护通过题目数量和冻结题目数量
            // 按照输入顺序覆盖最后一次提交
            last_submission_status_ = status_code;
            last_submission_time_ = submission_time;
            // 按照输入顺序覆盖指定状态提交
            status_submission_time_[status_code] = submission_time;
            if ((status_code == Accepted) && (!IsAccepted())) {
                accepted_submission_time_ = submission_time;
            } // 记录全局首次正确提交（无论是否冻结）
            Freeze(freezed_count); // 如果处于冻结状态，会走冻结路线
            // 如果未冻结，通过状态会计入0，否则，计入1
            bool previous_accepted_status = accepted_[0]; // 记录提交结算之前的**非冻结状态**下的通过状态
            accepted_[freezed_] |= (status_code == Accepted);
            // 如果未冻结，并且是错误提交，并且当前没有通过，才计入0
            // 如果已冻结，并且是错误提交，并且当前没有通过，才计入1
            unaccepted_submission_count_[freezed_] += ((status_code != Accepted) &
                                                       (!IsAccepted()));
            // 如果已冻结并且冻结**前**未通过，统计提交次数
            freezed_submission_count_ += (freezed_ & (!accepted_[0]));
            bool status_change = accepted_[0] ^ previous_accepted_status;
            accepted_count += status_change;
            return CalculateTimePenalty(status_change);
        } // 新增一个status_code类型于time时的提交
          // 返回本次提交以后题目罚时**增量**
        int Release(int &accepted_count, int &freezed_count) { // accepted_count和freezed_count用于与Team对接，维护通过题目数量和冻结题目数量
            if (is_freezed || accepted_[0] || (!freezed_)) return 0; 
            // 拒绝解冻的条件：
            // 1. 仍然处于封榜状态
            // 2. 封榜以外的时段已经通过
            // 3. 已经解冻
            --freezed_count;
            freezed_ = false;
            bool previous_accepted_status = accepted_[0]; // 记录解冻结算之前的**非冻结状态**下的通过状态
            accepted_[0] |= accepted_[1]; // 合并冻结期间正确状态
            unaccepted_submission_count_[0] += unaccepted_submission_count_[1]; // 合并冻结期间错误提交
            freezed_submission_count_ = 0; // 一旦解冻，该参数不再有任何意义
            accepted_count += accepted_[0] ^ previous_accepted_status;
            return CalculateTimePenalty(accepted_[0] ^ previous_accepted_status);
        } // 解冻结算
          // 返回本次解冻以后题目罚时**增量**
        std::pair<int, int> QueryAllSubmision() {
            if ((last_submission_status_ == -1) || (last_submission_time_ == -1)) {
                return std::make_pair(-1, -1);
            } // 没有此提交
            return std::make_pair(last_submission_status_, last_submission_time_);
        } // 返回最后一次任意状态的提交
        int QueryStatusSubmision(int status_code) {
            if (status_submission_time_[status_code] == -1) return -1;
            return status_submission_time_[status_code];
        }
        void PrintStatus() {
            if (!freezed_) {
                if (accepted_[0]) {
                    if (unaccepted_submission_count_[0] == 0) std::cout << "+"; // 一发过
                    else  std::cout << "+" << unaccepted_submission_count_[0]; // 没有一发过
                } // 通过了
                else {
                    if (unaccepted_submission_count_[0] == 0) std::cout << "."; // 没交题
                    else  std::cout << "-" << unaccepted_submission_count_[0]; // 交了，没过
                } // 没通过
            } // 未冻结，按照未冻结规则计算
            else {
                // 注意，只有Submit的时候才会试图freeze，所以此时freezed_submission_count_ > 0
                if (unaccepted_submission_count_[0] == 0) std::cout << "0/" << freezed_submission_count_; // 封榜前没有错误提交
                else std::cout << "-" << unaccepted_submission_count_[0] << "/" << freezed_submission_count_; // 封榜前有错误提交
            } // 冻结了，按照冻结规则结算（注意，已经通过的题目不会被冻结）
            return;
        } // 输出题目状态
        int QueryFirstAcceptedTime() {
            return accepted_submission_time_;
        } // 查询首次AC状态
        bool QueryActivateAcceptedStatus() {
            return accepted_[0];
        } // 查询非冻结AC状态
        bool IsFreezed() {
            return freezed_;
        }
}temporary_problem;
std::vector<Problem> problem_list; // 存储[题目数量]个初始化题目

/**
 * The class Team
 * @param name_:                         队伍名称
 * @param accepted_count_:               非冻结的过题数量
 * @param freezed_count_:                目前被冻结的题目数量
 * @param time_penalty_:                 非冻结的罚时
 * @param problem_:                      题目状态集
 * @param accepted_time_:                存储非解冻首次过题时间
 * @param ranking_:                      队伍的排名
 * @param accepted_:                     该题目是否已经以非解冻形式AC
 * @param last_submission_status_:       最后一次提交的状态
 * @param last_submission_time_:         最后一次提交的时间
 * @param status_submission_time_:       指定状态的最后一次提交
 * @param last_submission_problem_:      最后一次提交的题目
 * @param status_submission_problem_:    指定状态最后一次提交的题目
 * @param freezed_problem_code_:         被冻结的题目题号（只在滚榜时刷新）
 */
class Team{
    private:
        int freezed_count_;
        std::stack<int> freezed_problem_code_;
        std::vector<Problem> problem_;
        int ranking_;
        std::vector<bool> accepted_;
        int last_submission_status_, last_submission_time_, last_submission_problem_;
        int status_submission_time_[4], status_submission_problem_[4];
        bool FlushAcceptedTime(int problem_code) {
            if (!problem_[problem_code].QueryActivateAcceptedStatus()) return false; // 没有非解冻AC提交
            if (accepted_[problem_code]) return false; // 已经记录了非解冻AC提交
            accepted_[problem_code] = true;
            accepted_time_.push_back(problem_[problem_code].QueryFirstAcceptedTime());
            return true;
        } // 刷新problem_code的非解冻AC提交状态
        void SortAcceptedTime() {
            std::sort(accepted_time_.begin(), accepted_time_.end());
            return;
        } // 把accepted_time从小到大排序
    public:
        int accepted_count_;
        int time_penalty_;
        std::vector<int> accepted_time_;
        std::string name_;
        Team() : accepted_count_(0),
                 freezed_count_(0),
                 time_penalty_(0),
                 last_submission_status_(-1),
                 last_submission_time_(-1),
                 last_submission_problem_(-1) {
            name_.clear();
            problem_.clear();
            accepted_time_.clear();
            accepted_.clear();
            status_submission_time_[Accepted] = -1;
            status_submission_time_[Wrong_Answer] = -1;
            status_submission_time_[Runtime_Error] = -1;
            status_submission_time_[Time_Limit_Exceed] = -1;
            status_submission_problem_[Accepted] = -1;
            status_submission_problem_[Wrong_Answer] = -1;
            status_submission_problem_[Runtime_Error] = -1;
            status_submission_problem_[Time_Limit_Exceed] = -1;
            while (freezed_problem_code_.size()) freezed_problem_code_.pop();
            return;
        }
        bool IsFreezed() {
            return (freezed_count_ != 0);
        } // 如果有未解冻的题，那么返回true
        void SetRanking(int ranking) {
            ranking_ = ranking;
            return;
        }
        int QueryRanking() {
            return ranking_;
        }
        void QuerySubmission(int problem_code, int status) {
            if (problem_code == ALL) {
                if (status == ALL) {
                    if ((last_submission_status_ == -1) || (last_submission_time_ == -1) || (last_submission_problem_ == -1)) {
                        std::printf("Cannot find any submission.\n");
                    }
                    else {
                        std::cout << name_ << " " << static_cast<char>(last_submission_problem_ + 'A') << ' ' << SUBMISSION[last_submission_status_] << ' ' << last_submission_time_ << '\n';
                    }
                }
                else {
                    if ((status_submission_time_[status] == -1) || (status_submission_problem_[status] == -1)) {
                        std::printf("Cannot find any submission.\n");
                    }
                    else {
                        std::cout << name_ << " " << static_cast<char>(status_submission_problem_[status] + 'A') << ' ' << SUBMISSION[status] << ' ' << status_submission_time_[status] << '\n';
                    }
                }
            }
            else{   
                if (status == ALL) {
                    std::pair<int, int> submission = problem_[problem_code].QueryAllSubmision();
                    if ((submission.first == -1) || (submission.second == -1)) {
                        std::printf("Cannot find any submission.\n");
                    }
                    else {
                        std::cout << name_ << " " << static_cast<char>(problem_code + 'A') << ' ' << SUBMISSION[submission.first] << ' ' << submission.second << '\n';
                    }
                }
                else { 
                    int submission_time = problem_[problem_code].QueryStatusSubmision(status);
                    if (submission_time == -1) {
                        std::printf("Cannot find any submission.\n");
                    }
                    else {
                        std::cout << name_ << " " << static_cast<char>(problem_code + 'A') << ' ' << SUBMISSION[status] << ' ' << submission_time << '\n';
                    }
                }
            }
            return;
        }
        void SetName(const std::string &team_name) {
            name_ = team_name;
            return;
        }
        void Initialize() {
            problem_ = problem_list;
            for (int i = 0; i < problem_count; ++i) {
                accepted_.push_back(false);
            } // 默认都没有非解冻AC
            return;
        }
        bool DetermineEffective(int problem_code, int status_code, int submission_time) {
            if (is_freezed) return false; // 封榜提交
            if (status_code != Accepted) return false; // 错误提交
            if (accepted_[problem_code] || 
                problem_[problem_code].QueryActivateAcceptedStatus()) {
                return false; // 正确提交，但是已经记录了
            }
            return true;
        } // 判断该提交是否可能导致排行榜变化
        void Submit(int problem_code, int status_code, int submission_time) {
            // 按照输入顺序覆盖最后一次提交
            last_submission_status_ = status_code;
            last_submission_time_ = submission_time;
            last_submission_problem_ = problem_code;
            // 按照输入顺序覆盖指定状态提交
            status_submission_time_[status_code] = submission_time;
            status_submission_problem_[status_code] = problem_code;
            time_penalty_ += problem_[problem_code].Submit(status_code, submission_time, accepted_count_, freezed_count_);
            FlushAcceptedTime(problem_code);
            // 如果是冻结期间提交，那么Flush无疑是无效的
            // 如果是非冻结期间提交，那么会按时序从小到大压入AC时间，不需要排序
            return;
        }
        void PrintStatus() {
            std::cout << name_ << ' ' << ranking_ << ' ' << accepted_count_ << ' ' << time_penalty_ << ' ';
            for (int i = 0; i < problem_count; ++i) {
                problem_[i].PrintStatus();
                if (i < problem_count - 1) std::putchar(' ');
            }
            std::cout << '\n';
            return;
        }
        void PrepareRelease() {
            while (freezed_problem_code_.size()) freezed_problem_code_.pop();
            for (int i = problem_count - 1; i >= 0; --i) {
                if (problem_[i].IsFreezed()) {
                    freezed_problem_code_.push(i);
                } // 从大到小压栈
            }
            return;
        } // 准备接受解冻
        bool Release() {
            if (!IsFreezed()) return false;
            int problem_code = freezed_problem_code_.top();
            time_penalty_ += problem_[problem_code].Release(accepted_count_, freezed_count_);
            if (FlushAcceptedTime(problem_code)) {
                SortAcceptedTime(); // 因为按照编号顺序解冻，所以无法保证time有序，所以需要排序
            }
            freezed_problem_code_.pop();
            return IsFreezed();
        }
}temporary_team;
std::vector<Team> team_list;

auto comparator = [](const int &a, const int &b)->bool {
    if (team_list[a].accepted_count_ != team_list[b].accepted_count_) {
        return team_list[a].accepted_count_ > team_list[b].accepted_count_;
    }
    if (team_list[a].time_penalty_ != team_list[b].time_penalty_) {
        return team_list[a].time_penalty_ < team_list[b].time_penalty_;
    }
    int accepted_count = team_list[a].accepted_count_;
    for (int i = accepted_count - 1; i >= 0; --i) {
        if (team_list[a].accepted_time_[i] != team_list[b].accepted_time_[i]) {
            return team_list[a].accepted_time_[i] < team_list[b].accepted_time_[i];
        }
    }
    return team_list[a].name_ < team_list[b].name_;
}; // 判断a队排名是否比b队靠前
class Comparator{
    public:
        bool operator()(const int &a, const int &b) const {
            return comparator(a, b);
        }
}; // 判断a队排名是否比b队靠前

std::unordered_map<std::string, int> team_code_map;
std::vector<int> team_code_vector; // 用来维护ranking
int team_code_count = 0;

int main() {
    while (Execute()) ;
    std::printf("[Info]Competition ends.\n");
    return 0;
}

std::set<int, Comparator> all_team_code; // 实时榜单
std::set<int, Comparator> freezed_team_code;
// 命令检索与执行模块
void Flush() {
    int count = 0;
    for (auto it = all_team_code.begin(); it != all_team_code.end(); ++it) {
        team_code_vector[count++] = *it;
    }
    for (int i = 0; i < team_code_count; ++i) {
        team_list[team_code_vector[i]].SetRanking(i + 1);
    }
    return;
}

bool Execute() {
    std::string command_line, command_code, rubbish;
    getline(std::cin, command_line);
    std::stringstream sstr(command_line);
    sstr >> command_code;
    if (command_code == "ADDTEAM") {
        if (is_started) {
            std::printf("[Error]Add failed: competition has started.\n");
        }
        else {
            std::string team_name;
            sstr >> team_name;
            int new_code = ApplyTeamCode(team_name);
            if (new_code == -1) {
                std::printf("[Error]Add failed: duplicated team name.\n");
            }
            else {
                std::printf("[Info]Add successfully.\n");
                temporary_team.SetName(team_name);
                team_list.push_back(temporary_team);
                team_code_vector.push_back(new_code);
            }
        }
    }
    else if (command_code == "START") {
        if (is_started) {
            std::printf("[Error]Start failed: competition has started.\n");
        }
        else {
            is_started = true;
            sstr >> rubbish >> duration_time;
            sstr >> rubbish >> problem_count;
            problem_list.clear();
            for (int i = 0; i < problem_count; ++i) {
                problem_list.push_back(temporary_problem);
            } // 得到初始化的problem_list
            for (int i = 0; i < team_code_count; ++i) {
                team_list[i].Initialize();
                all_team_code.insert(i);
            }
            Flush();
            std::printf("[Info]Competition starts.\n");
        }
    }
    else if (command_code == "SUBMIT") {
        char problem_name;
        std::string team_name, submit_status;
        int team_code, problem_code, status_code, submission_time;
        sstr >> problem_name >> rubbish;
        problem_code = problem_name - 'A';
        sstr >> team_name >> rubbish;
        team_code = QueryTeamCode(team_name);
        sstr >> submit_status;
        if (submit_status == SUBMISSION[Accepted]) status_code = Accepted;
        else if (submit_status == SUBMISSION[Wrong_Answer]) status_code = Wrong_Answer;
        else if (submit_status == SUBMISSION[Runtime_Error]) status_code = Runtime_Error;
        else if (submit_status == SUBMISSION[Time_Limit_Exceed]) status_code = Time_Limit_Exceed;
        sstr >> rubbish >> submission_time;
        bool effective = team_list[team_code].DetermineEffective(problem_code, status_code, submission_time);
        if (effective) {
            all_team_code.erase(team_code);
        }
        team_list[team_code].Submit(problem_code, status_code, submission_time);
        if (effective) {
            all_team_code.insert(team_code);
        }
    }
    else if (command_code == "FLUSH") {
        Flush();
        std::printf("[Info]Flush scoreboard.\n");
    }
    else if (command_code == "FREEZE") {
        if (!is_freezed) {
            is_freezed = true;
            std::printf("[Info]Freeze scoreboard.\n");
        }
        else {
            std::printf("[Error]Freeze failed: scoreboard has been frozen.\n");
        }
    }
    else if (command_code == "SCROLL") {
        if (!is_freezed) {
            std::printf("[Error]Scroll failed: scoreboard has not been frozen.\n");
        }
        else {
            is_freezed = false;
            std::printf("[Info]Scroll scoreboard.\n");
            Flush();
            PrintRankingList();
            freezed_team_code.clear();
            for (int i = 0; i < team_code_count; ++i) {
                if (team_list[i].IsFreezed()) {
                    freezed_team_code.insert(i);
                    team_list[i].PrepareRelease();
                }
            }
            while (freezed_team_code.size()) {
                // 取出排名最大的
                auto it = freezed_team_code.end();
                --it; // it现在指向排名最大的队伍

                int team_code = *it, old_nxt;
                // 擦除team_code
                freezed_team_code.erase(it);

                auto all_it = all_team_code.find(team_code);
                // 记录team_code的后继
                ++all_it;
                if (all_it == all_team_code.end()) old_nxt = -1;
                else  old_nxt = *all_it;
                --all_it;
                // 擦除team_code
                all_team_code.erase(all_it);

                if (team_list[team_code].Release()) {
                    freezed_team_code.insert(team_code);
                } // 解冻，如果还没解完
                all_team_code.insert(team_code);
                int nxt;
                all_it = all_team_code.find(team_code);
                // 记录team_code的新后继
                ++all_it;
                if (all_it == all_team_code.end()) nxt = -1;
                else  nxt = *all_it;

                if (old_nxt == nxt) continue; // 滚榜没有改变排名
                // 如果改变了排名，只可能是排名上升，替代了后继的位置

                std::cout << team_list[team_code].name_ << ' ' <<
                             team_list[nxt].name_ << ' ' << 
                             team_list[team_code].accepted_count_ << ' ' << 
                             team_list[team_code].time_penalty_ << '\n';
            }
            Flush(); // 基于新数据刷榜
            PrintRankingList();
        }
    }
    else if (command_code == "QUERY_RANKING") {
        std::string team_name;
        sstr >> team_name;
        int team_code = QueryTeamCode(team_name);
        if (team_code == -1) {
            std::printf("[Error]Query ranking failed: cannot find the team.\n");
        }
        else {
            std::printf("[Info]Complete query ranking.\n");
            if (is_freezed) {
                std::printf("[Warning]Scoreboard is frozen. The ranking may be inaccurate until it were scrolled.\n");
            }
            std::cout << team_name << " NOW AT RANKING " << team_list[team_code].QueryRanking() << '\n';
        }
    }
    else if (command_code == "QUERY_SUBMISSION") {
        std::string team_name;
        int problem_code, status_code, team_code;
        sstr >> team_name >> rubbish;
        team_code = QueryTeamCode(team_name);
        sstr >> rubbish;
        if (rubbish.size() > 9) problem_code = ALL;
        else problem_code = rubbish[8] - 'A';
        sstr >> rubbish;
        sstr >> rubbish;
        if (rubbish == "STATUS=" + SUBMISSION[Accepted]) status_code = Accepted;
        else if (rubbish == "STATUS=" + SUBMISSION[Wrong_Answer]) status_code = Wrong_Answer;
        else if (rubbish == "STATUS=" + SUBMISSION[Runtime_Error]) status_code = Runtime_Error;
        else if (rubbish == "STATUS=" + SUBMISSION[Time_Limit_Exceed]) status_code = Time_Limit_Exceed;
        else status_code = ALL;
        if (team_code == -1) {
            std::printf("[Error]Query submission failed: cannot find the team.\n");
        }
        else {
            std::printf("[Info]Complete query submission.\n");
            team_list[team_code].QuerySubmission(problem_code, status_code);
        }
    }
    return (command_code != "END");
}

// 队伍编码
int QueryTeamCode(const std::string &team_name) {
    auto it = team_code_map.find(team_name);
    if (it == team_code_map.end()) {
        return -1;
    }
    return it->second;
}
int ApplyTeamCode(const std::string &team_name) {
    auto it = team_code_map.find(team_name);
    if (it != team_code_map.end()) {
        return -1;
    }
    return (team_code_map[team_name] = (team_code_count++));
}

// 输出排行榜
void PrintRankingList() {
    for (int i = 0; i < team_code_count; ++i) {
        team_list[team_code_vector[i]].PrintStatus();
    }
    return;
}