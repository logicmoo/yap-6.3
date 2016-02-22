/*************************************************************************
*									 *
*	 YAP Prolog 							 *
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		errors.yap						 *
  * comments:	error messages for YAP					 *
*									 *
*									 *
*									 *
*************************************************************************/


/** @defgroup YAPError Error Handling

@ingroup YAPControl

The error handler is called when there is an execution error or a
warning needs to be displayed. The handlers include a number of hooks
to allow user-control.

Errors are terms of the form:

   - error( domain_error( Domain, Culprit )`
   - error( evaluation_error( Expression, Culprit )`
   - error( existence_error( Object, Culprit )`
   - error( instantiation_error )`
   - error( permission_error( Error, Permission, Culprit)`
   - error( representation_error( Domain, Culprit )`
   - error( resource_error( Resource, Culprit )`
   - error( syntax_error( Error )`
   - error( system_error( Domain, Culprit )`
   - error( type_error( Type, Culprit )`
   - error( uninstantiation_error(  Culprit )`

@{

*/

:- system_module( '$_errors', [system_error/2], ['$Error'/1,
                                                 '$do_error'/2,
                                                 system_error/3,
                                                 system_error/2]).

:- use_system_module( '$messages', [file_location/2,
        generate_message/3,
        translate_message/3]).


/**
 * @pred system_error( +Error, +Cause)
 *
 * Generate a system error _Error_, informing the possible cause _Cause_.
 *
 */
system_error(Type,Goal) :- 
    '$do_error'(Type,Goal).


'$do_error'(Type,Goal) :-
%        format('~w~n', [Type]),    
	ancestor_location(Call, Caller),
	throw(error(Type, [
	       [g|g(Goal)],
	       [p|Call],
	       [e|Caller]])).

/**
 * @pred system_error( +Error, +Cause, +Culprit)
 *
 * Generate a system error _Error_, informing the source goal _Cause_ and a possible _Culprit_.
 *
 *
 * ~~~~~~~~~~
 * ~~~~~~~~~~
 *
 * 
 */
system_error(Type,Goal,Culprit) :- 
 %        format('~w~n', [Type]),
	ancestor_location(Call, Caller),
	throw(error(Type, [
	       [i|Culprit],
	       [g|g(Goal)],
	       [p|Call],
	       [e|Caller]])).

'$do_pi_error'(type_error(callable,Name/0),Message) :- !,
	'$do_error'(type_error(callable,Name),Message).
'$do_pi_error'(Error,Message) :- !,
	'$do_error'(Error,Message).

'$Error'(E) :-
	'$LoopError'(E,top).

'$LoopError'(_, _) :-
	flush_output(user_output),
	flush_output(user_error),
	fail.
'$LoopError'(Error, Level) :- !,
	'$process_error'(Error, Level),
	fail.
'$LoopError'(_, _) :-
	flush_output,
	fail.

'$process_error'('$abort', top) :- !,
	print_message(informational,abort(user)).
'$process_error'('$abort', _) :- !,
	throw('$abort').
'$process_error'(abort, top) :- !,
	print_message(informational,abort(user)).
'$process_error'(abort, _) :- !,
	throw(abort).
'$process_error'(error(thread_cancel(_Id), _G),top) :- !.
'$process_error'(error(thread_cancel(Id), G), _) :- !,
	throw(error(thread_cancel(Id), G)).
'$process_error'(error(permission_error(module,redefined,A),B), Level) :-
	Level \= top, !,
	throw(error(permission_error(module,redefined,A),B)).
'$process_error'(error(Msg, Where), _) :-
    print_message(error,error(Msg, Where)), !.
'$process_error'(Throw, _) :-
	print_message(error,error(unhandled_exception,Throw)).

%% @}