#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <string>

namespace CipherZ {
    using namespace eosio;
    using std::string;

    class Lottery : public contract {
        using contract::contract;
        
        public:
            Lottery(account_name self):contract(self) {}

            //@abi action
            void addstudent(const account_name account, uint64_t grade, uint64_t ssn, string firstname, string lastname) {
            require_auth(account);
            
            // Grade logic
            gradeMultiIndex grades(_self, _self);
            auto grade_iter = grades.find(grade);
            eosio_assert(grade_iter != grades.end(), "Grade must exist before adding a student");

            // Student insertion
            studentMultiIndex students(_self, _self);
            auto student_iter = students.find(ssn);
            eosio_assert(student_iter == students.end(), "student already exists");

            students.emplace(account, [&](auto& student) {
                student.key = students.available_primary_key();
                student.account_name = account;
                student.ssn = ssn;
                student.firstname = firstname;
                student.lastname = lastname;
                student.grade = grade;
            });

            }

            //@abi action
            void addgrade(const account_name account,uint64_t school, uint64_t grade_num, uint64_t openings) {
                require_auth(account);
                gradeMultiIndex grades(_self, _self);
                auto iterator = grades.find(grade_num);
                eosio_assert(iterator == grades.end(), "grade already exists");
                grades.emplace(account, [&](auto& _grade) {
                    _grade.key = grades.available_primary_key();
                    _grade.school = school;
                    _grade.account_name = account;
                    _grade.openings = openings;
                    _grade.grade_num = grade_num;
                    _grade.status = 0;
                });
            }

            //@abi action
            void updategrade(const account_name account, uint64_t key, uint64_t openings) {
                require_auth(account);
                gradeMultiIndex grades(_self, _self);
                auto iterator = grades.find(key);
                eosio_assert(iterator != grades.end(), "grade does not exist");
                grades.modify( iterator, _self, [&]( auto& _grade) {
                    _grade.openings = openings;
                });
            }

            //@abi action
            void updatestuden(const account_name account, uint64_t key, uint64_t ssn, string firstname, string lastname, uint64_t grade) {
                require_auth(account);
                 // Student insertion
                studentMultiIndex students(_self, _self);
                auto student_iter = students.find(key);
                eosio_assert(student_iter != students.end(), "student does not exist");

                students.modify( student_iter, _self, [&]( auto& student) {
                    student.firstname = firstname;
                    student.lastname = lastname;
                    student.grade = grade;
                    student.ssn = ssn;
                });
            }

            //@abi action
            void getstudent(const account_name account, const uint64_t key) {
                require_auth(account);
                studentMultiIndex students(_self, _self);
                auto iterator = students.find(key);
                eosio_assert(iterator != students.end(), "student not found");
                auto student = students.get(key);
                eosio_assert(student.account_name == account, "only parent can view student");
                print(" **SSN: ", student.ssn, 
                    " Key: ", student.key,
                    " First Name: ", student.firstname.c_str(), 
                    " Last Name: ", student.lastname.c_str(),
                    " Grade: ", student.grade,
                    " Result: ", student.result, "** ");
            }

            //@abi action
            void remstudent(const account_name account, const uint64_t key) {
                require_auth(account);

                studentMultiIndex students(_self, _self);
                auto iterator = students.find(key);
                eosio_assert(iterator != students.end(), "student not found");
                auto student = (*iterator);
                eosio_assert(student.account_name == account, "only parent can remove student");

                // Grade logic
                gradeMultiIndex grades(_self, _self);
                auto grade_index = grades.template get_index<N(bygrade)>();
                auto grade_iter = grade_index.find(student.grade);

                students.erase(iterator);

            }

            //@abi action
            void remgrade(const account_name account, const uint64_t key) {
                require_auth(account);
                gradeMultiIndex grades(_self, _self);
                auto iterator = grades.find(key);
                eosio_assert(iterator != grades.end(), "grade not found");
                auto grade = (*iterator);
                eosio_assert(grade.account_name == account, "only supervisor can remove grade");
                grades.erase(iterator);
            }

            //@abi action
            void getgrade(const account_name account, uint64_t key) {
                require_auth(account);
                gradeMultiIndex grades(_self, _self);
                auto iterator = grades.find(key);
                eosio_assert(iterator != grades.end(), "key does not exist");
                auto current_grade = (*iterator);
                print(" **Grade: ", current_grade.grade_num,
                        " Account: ", current_grade.account_name, 
                        " Openings: ", current_grade.openings, "** ");
            }

            //@abi action
            void getstudents(const account_name account) {
                require_auth(account);
                studentMultiIndex students(_self, _self);
                auto iterator = students.begin();
                eosio_assert(iterator != students.end(), "no students exists");
                while (iterator != students.end()) {
                    auto student = (*iterator);
                    print(" First Name: ", student.firstname.c_str(), 
                        " Last Name: ", student.lastname.c_str(),
                        " Grade: ", student.grade,
                        " Key: ", student.key,
                        " Result: ", student.result, "** ");
                    iterator++;
                }
            }

            //@abi action
            void getgrades(const account_name account, uint64_t school) {
                require_auth(account);
                gradeMultiIndex grades(_self, _self);
                auto school_index = grades.template get_index<N(byschool)>();
                auto school_iterator = school_index.find(school);
                while (school_iterator != school_index.end()) {
                    auto current_grade = (*school_iterator);
                    print(" **Grade: ", current_grade.grade_num, 
                    " Key: ", current_grade.key,
                    " School: ", current_grade.school,
                    " Status: ", current_grade.status,
                        " Openings: ", current_grade.openings, "** ");
                    school_iterator++;
                }
            }

            //@abi action
            void runlottery(account_name account, uint64_t school) {
                require_auth(account);
                studentMultiIndex students(_self, _self);
                auto student_index = students.template get_index<N(bygrade)>();
                gradeMultiIndex grades(_self, _self);
                auto school_index = grades.template get_index<N(byschool)>();
                auto grade_iter = school_index.find(school);
                while(grade_iter != school_index.end()) {
                    auto current_grade = (*grade_iter).grade_num;
                    auto student_iter = student_index.find(current_grade);
                    uint64_t result_index = 1;
                    while (student_iter != student_index.end()) {
                        auto current_student = (*student_iter);
                        if(current_student.grade == current_grade) {
                            student_index.modify(student_iter, account, [&](auto& student) {
                            student.result = result_index;
                        });
                        result_index++;
                        } 
                       student_iter++;
                    }
                 school_index.modify(grade_iter, account, [&](auto& grade) {
                    grade.status = 1;
                });   
                 grade_iter++;
                }
            }


        private:

            //@abi table student i64
            struct student {
                uint64_t account_name;
                uint64_t key;
                uint64_t ssn;
                string firstname;
                string lastname;
                uint64_t grade;
                uint64_t result;

                uint64_t primary_key() const { return ssn; }
                uint64_t grade_key() const { return grade; }
                uint64_t parent_key() const { return account_name; }

                EOSLIB_SERIALIZE(student, (account_name)(key)(ssn)(firstname)(lastname)(grade)(result));
            };

            typedef multi_index<N(student), student, 
            indexed_by<N(bygrade), const_mem_fun<student, uint64_t, &student::grade_key>>,
            indexed_by<N(byparent), const_mem_fun<student, uint64_t, &student::parent_key>>> studentMultiIndex;
    
            //@abi table grade i64
            struct grade {
                uint64_t account_name;
                uint64_t key;
                uint64_t school;
                uint64_t grade_num;
                uint64_t openings;
                uint64_t status;

                uint64_t primary_key() const { return key; }
                uint64_t grade_key() const { return grade_num; }
                uint64_t school_key() const { return school; }

                EOSLIB_SERIALIZE(grade, (account_name)(key)(school)(grade_num)(openings)(status))
            };

            // typedef multi_index<N(key), key> gradeMultiIndex;
            typedef multi_index<N(grade), grade, 
            indexed_by<N(byschool), const_mem_fun<grade, uint64_t, &grade::school_key>>,
            indexed_by<N(bygrade), const_mem_fun<grade, uint64_t, &grade::grade_key>>> gradeMultiIndex;

            //@abi table school i64
            struct school {
                uint64_t account_name;
                string schoolname;
                uint64_t status;

                string primary_key() const { return schoolname; }

                EOSLIB_SERIALIZE(school, (account_name)(schoolname)(status))
            };

            typedef multi_index<N(school), school> schoolIndex;
    };

    EOSIO_ABI(Lottery, (addstudent)(addgrade)(getstudents)(getgrades)(getstudent)(getgrade)(runlottery)(remstudent)(remgrade)(updategrade)(updatestuden))
}