/**
 * _________________________________________________
 *
 * @file   callback_ctx.hpp
 * @author Mustafa Kemal GILOR <mustafagilor@gmail.com>
 * @date   19.05.2022
 *
 * SPDX-License-Identifier:    MIT
 * _________________________________________________
 */

#pragma once

#include <tdslite/util/tdsl_inttypes.hpp>
#include <tdslite/util/tdsl_macrodef.hpp>
#include <tdslite/util/tdsl_type_traits.hpp>

namespace tdsl {

    // --------------------------------------------------------------------------------

    template <typename FNTYPE>
    struct function_signature;

    template <typename R, typename... Args>
    struct function_signature<R (*)(Args...)> {
        using type        = R (*)(Args...);
        using return_type = R;
    };

    template <typename T, typename FNTYPE>
    struct callback_context {
        void * user_ptr{nullptr}; // Will be passed to the callback function as first argument
        FNTYPE callback{nullptr}; // The callback function
        using function_type = FNTYPE;
        using return_type   = typename function_signature<FNTYPE>::return_type;

        template <typename R = return_type, typename... Args>
        inline auto maybe_invoke(Args &&... args) -> typename traits::enable_if<!traits::is_same<R, void>::value, tdsl::uint32_t>::type {
            return (callback ? callback(user_ptr, TDSLITE_FORWARD(args)...) : return_type{0});
        }

        template <typename R = return_type, typename... Args>
        inline auto maybe_invoke(Args &&... args) -> typename traits::enable_if<traits::is_same<R, void>::value, void>::type {
            if (callback) {
                callback(user_ptr, TDSLITE_FORWARD(args)...);
            }
        }
    };

    template <typename T, typename FNTYPE = tdsl::uint32_t (*)(/*user_ptr*/ void *, /*info type*/ const T &)>
    struct callback_context;

} // namespace tdsl