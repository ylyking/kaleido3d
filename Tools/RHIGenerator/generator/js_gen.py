# Javascript Binding Generator
from loader import ProtypeLoader
from name import NameUtil, TypeRegEx, make_enum_name, make_enum_value, make_struct_name, make_name_XXX_XXX, make_name_axxBxx, re_param
from c_type_extractor import extract_type

class JSGen(object):

    def __init__(self):
        print('')
        self.ns = ''
        self.type_tree = {}

    def gen(self, templ, outpath = ''):
        self.type_tree = templ.type_tree
        self.header = open('test.h', 'w')
        self.source = open('test.cpp', 'w')
        self.header.write('#ifndef __TEST__\n')
        self.header.write('#define __TEST__\n')
        self.header.write('#include <v8.h>\n')
        self.source.write('#include \"test.h\"\n')
        global_templ_handles = []
        header_function_entries = []
        source_functions = []
        for (ns, type_) in self.type_tree.items():
            for (t_name, t_node) in type_['interface'].items():
                handle_name = NameUtil['struct']('', t_name)
                global_templ_handles.append('\nv8::Global<v8::ObjectTemplate> g{0}Templ;'.format(handle_name))
                if 'functions' in t_node:
                    fns = t_node['functions']
                    for fn in fns:
                        if isinstance(fn, dict):
                            for (fn_name, fn_val) in fn.items():
                                v8_function_name = self.make_v8_method_name(ns, t_name, fn_name)
                                source_functions.append(self.make_function_impl(ns, t_name, fn_name, 
                                    fn_val['return'] if 'return' in fn_val else 'void', fn_val['params'] if 'params' in fn_val else None))
                                header_function_entries.append('\nvoid {0}(const v8::FunctionCallbackInfo<v8::Value>& args);'.format(v8_function_name))
                        else:
                            v8_function_name = self.make_v8_method_name(ns, t_name, fn)
                            header_function_entries.append('\nvoid {0}(const v8::FunctionCallbackInfo<v8::Value>& args);'.format(v8_function_name))
                            

        # write the header
        for header_func in header_function_entries:
            self.header.write(header_func)
        self.header.write('\n#endif')

        for handle in global_templ_handles:
            self.source.write(handle)

        for header_func in header_function_entries:
            self.source.write(header_func)

        for src_func in source_functions:
            self.source.write(src_func)
        
    def make_v8_method_name(self, ns, inf, fn):
        return '_'.join(' '.join([inf, fn]).split(' '))

    def travel_interface_by_name(self, name):
        for (ns, type_) in self.type_tree.items():
            if name in type_['interface']:
                return type_['interface'][name]
        return None
    
    def make_js_method_name(self, fn_name):
        return make_name_axxBxx(fn_name)

    def get_type_by_name(self, name):
        for (ns, type_) in self.type_tree.items():
            for (t_type, t_name) in type_.items():
                if name in t_name:
                    return t_type
        return name
    
    def get_struct_tree_by_name(self, name):
        for (ns, type_) in self.type_tree.items():
            if name in type_['struct']:
                return type_['struct'][name]
        return None

    def make_function_impl(self, ns, inf, function, ret, params):
        fn_header = '\nvoid {0}(const v8::FunctionCallbackInfo<v8::Value>& args)'.format(self.make_v8_method_name(ns, inf, function))
        fn_body = 'Local<External> holder = Local<External>::Cast(args.Holder()->GetInternalField(0));\n'
        fn_body += '  {0}* ptr = ({0}*)holder->Value();\n'.format(NameUtil['cpp_interface'](ns, inf))
        # expand args
        if params:
            self.expand_fn_args(params)

        (ret_t, ret_stat) = self.declare_ret_value(ns, ret, fn_body)
        if ret_stat:
            fn_body += ret_stat
        if ret_t == 'interface':
            '' # make template
            '' # new instance from template
        elif ret_t == 'enum':
            fn_body += '  args.GetReturnValue().Set(ret);'
        else:
            print ret_t

        return '{0}\n{{\n  {1}\n}}'.format(fn_header, fn_body)
    
    def get_real_type_name(self, type_name, ns=None, is_cpp=False):
        if self.is_basic_type(type_name):
            return type_name
        r_type_name = type_name
        real_param_t = re_param.findall(type_name)[0][2]
        tt_name = self.get_type_by_name(real_param_t)
        if real_param_t in self.type_tree[ns][tt_name]:
            param_t_name = tt_name
            real_param_t_name = NameUtil[(('cpp_' + param_t_name) if is_cpp else param_t_name)](self.ns if ns==None else ns, real_param_t)
            #r_type_name = re_param.sub(r'\1 ' + real_param_t_name, type_name) python 3.5
            r_type_name = TypeRegEx.sub('\g<annotate> ' + '\g<const>'+ real_param_t_name + '\g<pointer>', type_name)

        if r_type_name[0:1] == ' ':
            r_type_name = r_type_name[1:]

        return r_type_name

    def is_basic_type(self, type_name):
        return type_name in ['void', 'Float32', 'bool', 'uint32_t', 'Bool32', 'Float32x4', 'char', 'uint64_t']
    
    def is_type_need_declare(self, type_name):
        return type_name not in ['void']

    def declare_ret_value(self, ns, ret, fn_body):
        if self.is_type_need_declare(ret):
            # make template for ret type
            (is_const, ret_raw_type, is_pointer) = extract_type(ret)
            ret_t = self.get_type_by_name(ret_raw_type)
            #print ret_raw_type
            real_ret_t = self.get_real_type_name(ret_raw_type, ns, True)
            ret_stat = '// declare return value\n  {0} ret;\n'.format(real_ret_t)
            return ret_t, ret_stat 
        else:
            (is_const, ret_raw_type, is_pointer) = extract_type(ret)
            return self.get_type_by_name(ret_raw_type), None

    def expand_fn_args(self, params):
        for i in range(0, len(params)):
            param = params[i]
            if not param or 'type' not in param:
                continue
            p_type = param['type']
            p_name = param['name']
            (is_const, p_raw_type, is_pointer) = extract_type(p_type)
            p_type_type = self.get_type_by_name(p_raw_type)
            struct_stack = []
            if p_type_type == 'struct':
                struct_stack.append(p_raw_type)
                # DFS expand args
                arg_len = '->->'
                while len(struct_stack) > 0:
                    type_top = struct_stack.pop()
                    arg_len = arg_len[0:len(arg_len)-2]
                    # expand struct, declare a struct instance
                    struct_val = self.get_struct_tree_by_name(type_top)
                    if 'members' in struct_val:
                        struct_members = struct_val['members']
                        for member_val in struct_members:
                            (m_is_const, m_type, m_is_pointer) = extract_type(member_val['type'])
                            if self.get_type_by_name(m_type) == 'struct':
                                struct_stack.append(m_type)
                                arg_len = arg_len + '->'
                                #print(struct_stack)
                            else:
                                pass
            elif p_type_type == 'interface':
                pass

protype = ProtypeLoader('ngfx.yml')
jsGen = JSGen()
jsGen.gen(protype)




