#pragma once

namespace Engine {
namespace Log {

    struct log_error_t {

        template <typename Rec>
        struct receiver : Execution::algorithm_receiver<Rec> {

            template <typename... R>
            void set_error(R &&...errors)
            {
                ([this](R &error) {
                    LogDummy out { MessageType::ERROR_TYPE, get_file_name(error), get_line_nr(error), get_log(this->mRec) };
                    if constexpr (requires(std::ostream & o) { o << error; }) {
                        out << error;
                    } else {
                        out << "<" << typeid(R).name() << ">";
                    }
                }(errors),
                    ...);
                //this->mRec.set_error(std::forward<R>(errors)...); 
                this->mRec.set_done();//Is this ok?
            }
        };

        template <typename Sender>
        struct sender : Execution::algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
            {
                return Execution::algorithm_state<Sender, receiver<Rec>> { std::forward<Sender>(sender.mSender), std::forward<Rec>(rec) };
            }            
        };

        template <typename Sender>
        friend auto tag_invoke(log_error_t, Sender &&inner)
        {
            return sender<Sender> { { {}, std::forward<Sender>(inner) } };
        }

        template <typename Sender>
        requires tag_invocable<log_error_t, Sender>
        auto operator()(Sender &&sender) const
            noexcept(is_nothrow_tag_invocable_v<log_error_t, Sender>)
                -> tag_invoke_result_t<log_error_t, Sender>
        {
            return tag_invoke(*this, std::forward<Sender>(sender));
        }

        auto operator()() const
        {
            return pipable_from_right(*this);
        }
    };

    inline constexpr log_error_t log_error;

    inline constexpr auto with_log = [](Log *log) {
        return Execution::with_query_value(get_log, std::move(log));
    };
    
}
}