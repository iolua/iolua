/**
 *
 * @file     io_operation
 * @brief    Copyright (C) 2015  yayanyang All Rights Reserved
 * @author   yayanyang
 * @date     2015/12/07
 */
#ifndef LEMON_IO_REACTIVE_OP_HPP
#define LEMON_IO_REACTIVE_OP_HPP

#include <cstddef>
#include <system_error>
#include <lemon/nocopy.hpp>

namespace lemon{
    namespace io{

        class reactor_op : private nocopy
        {
        public:
            reactor_op                 *next;

            virtual ~reactor_op()
            {

            }

        protected:

            using action_f = bool(*)(reactor_op*);
            using complete_f = void(*)(reactor_op*);

            reactor_op(action_f action,complete_f complete)
                    :next(nullptr)
                    ,_action(action)
                    ,_complete(complete)

            {
                
            }

        public:
            bool action()
            {
                
                return _action(this);
            }

            void complete()
            {
                _complete(this);
            }

        private:

            action_f                    _action;
            complete_f                  _complete;
        };
    }
}


#endif //LEMON_IO_REACTIVE_OP_HPP