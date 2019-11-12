#ifdef RUBYINTERFACE

#include "ruby.h"
#include "exhaust.h"
#include "sim.h"
#include "insn_help.h"


VALUE cMars;


static VALUE mars_parse(VALUE self, VALUE fileName) {
    warrior_struct* w;
    mars_t* mars;
    VALUE* instructions;
    VALUE rWarrior;
    VALUE rInstructions;
    int i;

    /* bail out if filename is no string */
    Check_Type(fileName, T_STRING);

    Data_Get_Struct(self, mars_t, mars);
    
    /* init pmars warrior data structure */
    w = (warrior_struct*)MALLOC(sizeof(warrior_struct));
    memset(w, 0, sizeof(warrior_struct));
    w->fileName = STR2CSTR(fileName);   
    if (assemble_warrior(mars, w->fileName, w)) {
        printf("we have a problem.\n");
    }
    
    /* generate ruby data structure */
    instructions = (VALUE*)MALLOC(sizeof(VALUE)*w->instLen);
    rWarrior = rb_eval_string("wLocal = Warrior.new");
    rb_iv_set(rWarrior, "@name", rb_str_new2(w->name));
    rb_iv_set(rWarrior, "@authorName", rb_str_new2(w->authorName));
    rb_iv_set(rWarrior, "@startPos", INT2FIX(w->offset));   
    rb_iv_set(rWarrior, "@fileName", fileName);

    rInstructions = rb_ary_new();

    for (i=0; i < w->instLen; ++i) {
        mem_struct* cell = w->instBank + i;
        FIELD_T opcode = ((FIELD_T) (cell->opcode & 0xf8)) >> 3;
        FIELD_T modifier = (cell->opcode & 0x07);
        char instrStr[256];
        VALUE rInstruction;
        char replaceOpname[5];


        if (opcode == CMP) {
            sprintf(replaceOpname, opname[SEQ]);
        }
        else sprintf(replaceOpname, opname[opcode]);
        
        /* build new instruction */
        sprintf(instrStr, "Instruction.new(\"%s\", \"%s\", \"%c\", %d, \"%c\", %d)",
                replaceOpname,
                modname[modifier],
                PM_INDIR_A(cell->A_mode) ? addr_sym[INDIR_A_TO_SYM(cell->A_mode)] : addr_sym[cell->A_mode],
                cell->A_value,
                PM_INDIR_A(cell->B_mode) ? addr_sym[INDIR_A_TO_SYM(cell->B_mode)] : addr_sym[cell->B_mode],
                cell->B_value);
        rb_ary_push(rInstructions, rb_eval_string(instrStr));
    }   
    rb_iv_set(rWarrior, "@instructions", rInstructions);
    FREE(instructions);
    FREE(w->name);
    FREE(w->version);
    FREE(w->date);
    FREE(w->authorName);
    FREE(w->instBank);
    FREE(w);
    return rWarrior;
}

VALUE mars_new(VALUE rClass,
               VALUE rWarriors, 
               VALUE rCoreSize, 
               VALUE rCycles, 
               VALUE rMaxProcesses, 
               VALUE rMaxWarriorLen, 
               VALUE rMinWarriorDistance) 
{
    mars_t* mars;
    VALUE rMarsData;
    
    Check_Type(rCycles, T_FIXNUM);
    Check_Type(rWarriors, T_FIXNUM);
    Check_Type(rCoreSize, T_FIXNUM);
    Check_Type(rCycles, T_FIXNUM);
    Check_Type(rMaxProcesses, T_FIXNUM);
    Check_Type(rMaxWarriorLen, T_FIXNUM);
    Check_Type(rMinWarriorDistance, T_FIXNUM);
    
    mars = (mars_t*)malloc(sizeof(mars_t));
    memset(mars, 0, sizeof(mars_t));
        
    mars->seed = rng((s32_t)time(0)*0x1d872b41);
    mars->rounds = 1;
    mars->cycles = FIX2INT(rCycles);
    mars->coresize = FIX2INT(rCoreSize);
    mars->processes = FIX2INT(rMaxProcesses);
    mars->maxWarriorLength = FIX2INT(rMaxWarriorLen);
    mars->minsep = FIX2INT(rMinWarriorDistance);
    mars->nWarriors = FIX2INT(rWarriors);
    /* pmars */
    mars->errorcode = SUCCESS;
    mars->errorlevel = WARNING;
    mars->saveOper = 0;
    
    sim_alloc_bufs(mars);
    
    mars->errmsg[0] = '\0';                /* reserve for future */

    rMarsData = Data_Wrap_Struct(rClass, 0, sim_free_bufs, mars);   
    rb_obj_call_init(rMarsData, 0, 0);
    return rMarsData;
}

VALUE mars_new_standard(VALUE rClass, VALUE rWarriors) {
    return mars_new(rClass, 
                    rWarriors, 
                    INT2FIX(8000),
                    INT2FIX(80000), 
                    INT2FIX(8000), 
                    INT2FIX(100), 
                    INT2FIX(100));
}

VALUE mars_new_tiny(VALUE rClass, VALUE rWarriors) {
    return mars_new(rClass, 
                    rWarriors, 
                    INT2FIX(800), 
                    INT2FIX(8000), 
                    INT2FIX(800), 
                    INT2FIX(20), 
                    INT2FIX(20));
}
VALUE mars_initialize(VALUE self) {
    mars_t* mars;
    int rounds = 1;
    rb_iv_set(self, "@rounds", INT2FIX(rounds));
    Data_Get_Struct(self, mars_t, mars);
    mars->rounds = rounds;
    return self;
}

VALUE mars_get_rounds(VALUE self) {
    return rb_iv_get(self, "@rounds");
}

VALUE mars_get_coresize(VALUE self) {
    mars_t* mars;
    Data_Get_Struct(self, mars_t, mars);
    return INT2FIX(mars->coresize);
}

VALUE mars_get_cycles(VALUE self) {
    mars_t* mars;
    Data_Get_Struct(self, mars_t, mars);
    return INT2FIX(mars->cycles);
}

VALUE mars_get_processes(VALUE self) {
    mars_t* mars;
    Data_Get_Struct(self, mars_t, mars);
    return INT2FIX(mars->processes);
}

VALUE mars_get_length(VALUE self) {
    mars_t* mars;
    Data_Get_Struct(self, mars_t, mars);
    return INT2FIX(mars->maxWarriorLength);
}


VALUE mars_get_distance(VALUE self) {
    mars_t* mars;
    Data_Get_Struct(self, mars_t, mars);
    return INT2FIX(mars->minsep);
}


VALUE mars_get_warriors(VALUE self) {
    mars_t* mars;
    Data_Get_Struct(self, mars_t, mars);
    return INT2FIX(mars->nWarriors);
}

VALUE mars_set_rounds(VALUE self, VALUE newVal) {
    mars_t* mars;
    Check_Type(newVal, T_FIXNUM);
    rb_iv_set(self, "@rounds", newVal);
    Data_Get_Struct(self, mars_t, mars);
    mars->rounds = NUM2INT(newVal);
    return self;
}

VALUE mars_fight(int argc, VALUE* argv, VALUE self) {
    int i, j, k;
    int seed;
    mars_t* mars;

    Data_Get_Struct(self, mars_t, mars);
    if (argc != mars->nWarriors) {
        rb_raise(rb_eArgError, "You have to supply %d warriors instead of %d.", mars->nWarriors, argc);
    }   

    /* convert */
    for (i=0; i<argc; ++i) {        
        warrior_t* warrior = &(mars->warriors[i]);
        insn_t* in = NULL;
        VALUE rInstructionsArray = rb_iv_get(argv[i], "@instructions");
        VALUE rStartPos = rb_iv_get(argv[i], "@startPos");

        Check_Type(rInstructionsArray, T_ARRAY);
        Check_Type(rStartPos, T_FIXNUM);
        warrior->start = NUM2INT(rStartPos);
        warrior->len = RARRAY(rInstructionsArray)->len;
        warrior->have_pin = 0; /* TODO! */
        in = warrior->code;
        
        for (j=0; j<warrior->len; ++j) {
            int op, m, ma, mb;
            int flags = 0;
            VALUE rTmp;
            int tmp;
            
            VALUE rInsArray = rb_iv_get(RARRAY(rInstructionsArray)->ptr[j], "@data");
            Check_Type(rInsArray, T_ARRAY);
            
            if (j == warrior->start) flags |= fl_START;
            rTmp = RARRAY(rInsArray)->ptr[0];
            Check_Type(rTmp, T_FIXNUM);         
            op = NUM2INT(rTmp);

            rTmp = RARRAY(rInsArray)->ptr[1];
            Check_Type(rTmp, T_FIXNUM);         
            m = NUM2INT(rTmp);

            rTmp = RARRAY(rInsArray)->ptr[2];
            Check_Type(rTmp, T_FIXNUM);         
            ma = NUM2INT(rTmp);
            
            rTmp = RARRAY(rInsArray)->ptr[4];
            Check_Type(rTmp, T_FIXNUM);         
            mb = NUM2INT(rTmp);
            
            rTmp = RARRAY(rInsArray)->ptr[3];
            Check_Type(rTmp, T_FIXNUM);
            tmp = NUM2INT(rTmp);
            in->a = MODS(tmp, mars->coresize);
            
            rTmp = RARRAY(rInsArray)->ptr[5];
            Check_Type(rTmp, T_FIXNUM);
            tmp = NUM2INT(rTmp);
            in->b = MODS(tmp, mars->coresize);

            in->in = (flags << flPOS) | OP( op, m, ma, mb );
            ++in;
        }
    }
    
    check_sanity(mars);
    clear_results(mars);
    /* seed = mars->fixedPosition ? mars->fixedPosition - mars->minsep : rng((s32_t)time(0)*0x1d872b41); */
    if (mars->fixedPosition) {
        seed = mars->fixedPosition - mars->minsep;
    } else {
        seed = rng(mars->seed);
    }

    save_pspaces(mars);
    amalgamate_pspaces(mars);   /* Share P-spaces with equal PINs */

    /* Fight rounds rounds. */
    for (i=0; i < mars->rounds; ++i) {
        int nalive;
        sim_clear_core(mars);

        seed = compute_positions(seed, mars);
        load_warriors(mars);
        set_starting_order(i, mars);

        nalive = sim_mw(mars, mars->startPositions, mars->deaths);
        if (nalive<0)
            panic("simulator panic!\n");    

        accumulate_results(mars);
    }
    mars->seed = seed;

    /*
      output_results(mars); 
    */
    
    /* TODO: result */
    return self;
}

VALUE mars_results_wins(VALUE self, VALUE rWarriorNr, VALUE rSurvivedCount) {
    int warriorNr;
    int survivedCount;
    mars_t* mars;
    VALUE rRetval;
    
    Check_Type(rWarriorNr, T_FIXNUM);
    Check_Type(rSurvivedCount, T_FIXNUM);   
    warriorNr = NUM2INT(rWarriorNr);
    survivedCount = NUM2INT(rSurvivedCount);
    Data_Get_Struct(self, mars_t, mars);
    
    if (warriorNr < 0 || warriorNr >= mars->nWarriors) {
        rb_raise(rb_eRangeError, "warrior nr has to be within range [0, %d], but it is %d.", mars->nWarriors-1, warriorNr);
    }
    if (survivedCount < 1 || survivedCount > mars->nWarriors) {
        rb_raise(rb_eRangeError, "survived counter has to be within range [1, %d], but it is %d.", mars->nWarriors, warriorNr);
    }

    rRetval = INT2FIX(mars->results[warriorNr*(mars->nWarriors+1) + survivedCount]);
    return rRetval;
}

VALUE mars_results_losses(VALUE self, VALUE rWarriorNr) {
    int warriorNr;
    VALUE rRetval;
    mars_t* mars;

    Check_Type(rWarriorNr, T_FIXNUM);
    warriorNr = NUM2INT(rWarriorNr);
    Data_Get_Struct(self, mars_t, mars);

    if (warriorNr < 0 || warriorNr >= mars->nWarriors) {
        rb_raise(rb_eRangeError, "warrior nr has to be within range [0, %d], but it is %d.", mars->nWarriors-1, warriorNr);
    }
    
    rRetval = INT2FIX(mars->results[warriorNr*(mars->nWarriors+1) + 0]);
    return rRetval;
}

VALUE mars_results_str(VALUE self) {
    char str[32];
    mars_t* mars;
    VALUE rStr;

    Data_Get_Struct(self, mars_t, mars);

    if (mars->nWarriors == 2 && !mars->isMultiWarriorOutput) {
        sprintf(str, "%ld %ld\n", mars->results[0*(mars->nWarriors+1) + 1], mars->results[0*(mars->nWarriors+1) + 2]);
        rStr = rb_str_new(str, strlen(str));
        sprintf(str, "%ld %ld\n", mars->results[1*(mars->nWarriors+1) + 1], mars->results[1*(mars->nWarriors+1) + 2]);
        rStr = rb_str_concat(rStr, rb_str_new(str, strlen(str)));
    } else {
        int i;
        rStr = rb_str_new("", 0);
        for (i=0; i<mars->nWarriors; i++) {
            int j;
            for (j=1; j<=mars->nWarriors; j++) {                
                sprintf(str, "%ld ", mars->results[i*(mars->nWarriors+1) + j]);
                rStr = rb_str_concat(rStr, rb_str_new(str, strlen(str)));
            }
            sprintf(str, "%ld\n", mars->results[i*(mars->nWarriors+1) + 0]);
            rStr = rb_str_concat(rStr, rb_str_new(str, strlen(str)));
        }
    }
    return rStr;
}

VALUE mars_score(VALUE self, VALUE rWarriorNr) {
    int warriorNr;
    int survivors;
    VALUE rScore;
    mars_t* mars;
    unsigned long score;

    Check_Type(rWarriorNr, T_FIXNUM);
    warriorNr = NUM2INT(rWarriorNr);
    Data_Get_Struct(self, mars_t, mars);

    if (warriorNr < 0 || warriorNr >= mars->nWarriors) {
        rb_raise(rb_eRangeError, "warrior nr has to be within range [0, %d], but it is %d.", mars->nWarriors-1, warriorNr);
    }
    
    score = 0;
    if (rb_block_given_p()) {
        VALUE rParams = rb_ary_new2(2);
        rb_ary_store(rParams, 0, INT2NUM(mars->nWarriors));
        for (survivors=1; survivors <= mars->nWarriors; ++survivors) {      
            VALUE rYieldResult;
            rb_ary_store(rParams, 1, INT2NUM(survivors));
            rYieldResult = rb_yield(rParams);
            Check_Type(rYieldResult, T_FIXNUM);
            
            score += mars->results[warriorNr*(mars->nWarriors+1) + survivors] * FIX2INT(rYieldResult);
        }
    }
    else {
        for (survivors=1; survivors <= mars->nWarriors; ++survivors) {
            score += (long)(mars->results[warriorNr*(mars->nWarriors+1) + survivors]) * (long)((mars->nWarriors*mars->nWarriors-1)/survivors);
        }
    }
    return INT2NUM(score);
}

VALUE mars_scores(VALUE self) {
    VALUE rScores;
    mars_t* mars;
    int i;

    Data_Get_Struct(self, mars_t, mars);
    rScores = rb_ary_new2(mars->nWarriors);
    for (i=0; i<mars->nWarriors; ++i) {
        rb_ary_store(rScores, i, mars_score(self, INT2FIX(i)));
    }
    return rScores;
}

void Init_Mars() {
    if (!rb_require("Warrior")) {
        printf("can not load Warrior.\n");
    }
    cMars = rb_define_class("Mars", rb_cObject);
    rb_define_singleton_method(cMars, "new", mars_new, 6);
    rb_define_singleton_method(cMars, "standard", mars_new_standard, 1);
    rb_define_singleton_method(cMars, "tiny", mars_new_tiny, 1);

    rb_define_method(cMars, "rounds", mars_get_rounds, 0);
    rb_define_method(cMars, "coresize", mars_get_coresize, 0);
    rb_define_method(cMars, "cycles", mars_get_cycles, 0);
    rb_define_method(cMars, "max_processes", mars_get_processes, 0);
    rb_define_method(cMars, "max_length", mars_get_length, 0);
    rb_define_method(cMars, "min_distance", mars_get_distance, 0);
    rb_define_method(cMars, "warriors", mars_get_warriors, 0);
    rb_define_method(cMars, "cycles", mars_get_cycles, 0);

    rb_define_method(cMars, "rounds=", mars_set_rounds, 1);

    rb_define_method(cMars, "initialize", mars_initialize, 0);

    rb_define_method(cMars, "parse", mars_parse, 1);
    rb_define_method(cMars, "fight", mars_fight, -1);
    rb_define_method(cMars, "results_str", mars_results_str, 0);
    /* rb_define_method(cMars, "results", mars_results, 0); */

    rb_define_method(cMars, "wins", mars_results_wins, 2);
    rb_define_method(cMars, "losses", mars_results_losses, 1);
    
    rb_define_method(cMars, "score", mars_score, 1);
    rb_define_method(cMars, "scores", mars_scores, 0);
}

#endif
