# SPDX-FileCopyrightText: 2016 Stephen Kelly <steveire@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

import os, sys

import rules_engine
sys.path.append(os.path.dirname(os.path.dirname(rules_engine.__file__)))
import Qt5Ruleset

def local_function_rules():
    return [
        ["KAuth::ActionReply", "ActionReply", ".*", ".*", ".*int.*", rules_engine.function_discard],
    ]

class RuleSet(Qt5Ruleset.RuleSet):
    def __init__(self):
        Qt5Ruleset.RuleSet.__init__(self)
        self._fn_db = rules_engine.FunctionRuleDb(lambda: local_function_rules() + Qt5Ruleset.function_rules())
