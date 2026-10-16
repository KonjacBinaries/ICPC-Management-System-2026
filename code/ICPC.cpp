// ICPC Management System
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <cstdio>

const std::string SUBMISSION[4] = {"Accepted", "Wrong_Answer", "Runtime_Error", "Time_Limit_Exceed"};
constexpr int Accepted = 0;
constexpr int Wrong_Answer = 1;
constexpr int Runtime_Error = 2;
constexpr int Time_Limit_Exceed = 3;
bool is_freezed = false; // 是否封榜

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
        int CalculateTimePenalty() {
            if (accepted_[0]) {
                return 20 * unaccepted_submission_count_[0] + accepted_submission_time_;
            } // 未冻结的时候，已经通过，按照通过计算
            // 值得注意的是，accepted_submission_time_即便在冻结期间也会更新
            // 但是这不存在问题，因为如果accepted_[0] = true，
            // 那么accepted_submission_time_记录的时间必然早于冻结时间

            // 未冻结的时候，没有通过，按照不通过计算
            return 20 * unaccepted_submission_count_[0];
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
        int Submit(int status_code, int time, int &accepted_count, int &freezed_count) { // accepted_count和freezed_count用于与Team对接，维护通过题目数量和冻结题目数量
            // 按照输入顺序覆盖最后一次提交
            last_submission_status_ = status_code;
            last_submission_time_ = time;
            // 按照输入顺序覆盖指定状态提交
            status_submission_time_[status_code] = time;
            if ((status_code == Accepted) && (!IsAccepted())) {
                accepted_submission_time_ = time;
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
            accepted_count += accepted_[0] ^ previous_accepted_status;
            return CalculateTimePenalty();
        } // 新增一个status_code类型于time时的提交
          // 返回本次提交以后题目罚时
        int Release(int &accepted_count, int &freezed_count) { // accepted_count和freezed_count用于与Team对接，维护通过题目数量和冻结题目数量
            if (is_freezed || accepted_[0] || (!freezed_)) return CalculateTimePenalty(); 
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
            return CalculateTimePenalty();
        } // 解冻结算
          // 返回(本次解冻是否 **导致** 了 **非冻结状态下的** 通过, 本次解冻以后题目罚时)
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
        std::string Status() {
            std::stringstream sstr;
            sstr.clear();
            if (!freezed_) {
                if (accepted_[0]) {
                    if (unaccepted_submission_count_[0] == 0) sstr << "+"; // 一发过
                    else  sstr << "+" << unaccepted_submission_count_[0]; // 没有一发过
                } // 通过了
                else {
                    if (unaccepted_submission_count_[0] == 0) sstr << "."; // 没交题
                    else  sstr << "-" << unaccepted_submission_count_[0]; // 交了，没过
                } // 没通过
            } // 未冻结，按照未冻结规则计算
            else {
                // 注意，只有Submit的时候才会试图freeze，所以此时freezed_submission_count_ > 0
                if (unaccepted_submission_count_[0] == 0) sstr << "0/" << freezed_submission_count_; // 封榜前没有错误提交
                else sstr << "-" << unaccepted_submission_count_[0] << "/" << freezed_submission_count_; // 封榜前有错误提交
            } // 冻结了，按照冻结规则结算（注意，已经通过的题目不会被冻结）
            return sstr.str();
        } // 返回题目状态
};

std::unordered_map<std::string, int> team_code_map;
int team_code_count = 0;

int QueryTeamCode(const std::string &team_name);
void AddTeam(int team_code);
void StartGame(int duration_time, int problem_count);
void Submit(char problem_name, int team_code, int status_code, int time);
void Flush();
void Freeze();
void Scroll(); // 如果解冻以后通过了新题，那么排名要么上升要么不变；如果解冻以后没有通过新题，那么排名要么下降要么不变
void QueryRanking(int team_code);
void QuerySubmission(int team_code, char problem_name, int status_code);
void End();
bool Execute();
int main() {
    
    // while (Execute()) ;
    return 0;
}

// 命令检索与执行模块
bool Execute() {
    std::string command_line;
    command_line.clear();
    std::cin.clear();
    getline(std::cin, command_line);
    std::stringstream sstr(command_line);
    std::string command_code;
    command_code.clear();
    sstr >> command_code;
    if (command_code == "ADDTEAM") {
    }
    else if (command_code == "START") {
    }
    else if (command_code == "SUBMIT") {
    }
    else if (command_code == "FLUSH") {
    }
    else if (command_code == "FREEZE") {
    }
    else if (command_code == "SCROLL") {
    }
    else if (command_code == "QUERY_RANKING") {
    }
    else if (command_code == "QUERY_SUBMISSION") {
    }
    return (command_code != "END");
}

// 队伍编码
int QueryTeamCode(const std::string &team_name) {
    auto it = team_code_map.find(team_name);
    if (it == team_code_map.end()) {
        team_code_map[team_name] = ++team_code_count;
        return team_code_count;
    }
    return it->second;
}