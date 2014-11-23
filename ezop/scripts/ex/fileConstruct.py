#coding: utf-8

import os, sys, re, codecs
from xml.dom import minidom

import parsePro
import config

#SMALL='small'
#FULL='full'
#
#ENC_IN = 'cp1251'
#ENC_OUT = 'utf-8'

class ontology:
    def __init__(self, title):
        self.title = title
        self.temps = {}
        self.in_temps = {}
        self.elems = []
        self.obs = []
        self.res_ops = []
        self.mors = {}
        self.appl = {}
    
    def fullremake(self):
        for i in self.elems + self.obs:# + self.res_ops:
            i.remake(self.temps, self.in_temps)
        for t in self.in_temps:
            self.in_temps[t].remake(self.temps, self.in_temps)
            
    def out(self, what):
        dw = {'elems': self.elems, 'obs': self.obs, 'res_ops':self.res_ops}
        what_iter = dw[what]
    
    def find_mor(ont):
        el_mor = []        
        mymor = 'отображениe'
        myattr = 'свойства'
        
        for i in range(0, len(ont.elems)):
            if ont.elems[i].e_ob == myattr:
                tmp = mor(ont.elems[i].e_elem, ont.elems[i].e_conceptId, False)
                ont.mors[tmp.name] = tmp
                ont.elems[i].e_isMor = True
            elif ont.elems[i].e_ob == mymor:
                tmp = mor(ont.elems[i].e_elem, ont.elems[i].e_conceptId, True)
                ont.mors[tmp.name] = tmp
                ont.elems[i].e_isMor = True
            
        temp_mor = []
        for i in ont.in_temps:
            tmp = ont.in_temps[i]
            
            if tmp.it_restype == myattr and '@' not in tmp.it_text:
                tmp_m = mor(tmp.it_text, tmp.it_conceptId, False)
                ont.mors[tmp_m.name] = tmp_m
                tmp.it_isMor = True
                
            elif tmp.it_restype == mymor and '@' not in tmp.it_text:
                tmp_m = mor(tmp.it_text, tmp.it_conceptId, True)
                ont.mors[tmp_m.name] = tmp_m
                tmp.it_isMor = True
            
    def fill_mors(self):
        for i in self.res_ops:            
            if i.ro_left.children[0].value == 'dom':
                mor_name = get_string(i.ro_left.children[1].children[0], self.temps, self.in_temps)
                mor_dom = get_string(i.ro_right, self.temps, self.in_temps)
                if mor_name not in self.mors:
                    self.mors[mor_name] = mor(mor_name, i.ro_conceptId, True)
                self.mors[mor_name].dom = mor_dom
                
            elif i.ro_left.children[0].value == 'cod':
                mor_name = get_string(i.ro_left.children[1].children[0], self.temps, self.in_temps)
                mor_cod = get_string(i.ro_right, self.temps, self.in_temps)
                if mor_name not in self.mors:
                    self.mors[mor_name] = mor(mor_name, i.ro_conceptId, True)
                self.mors[mor_name].cod = mor_cod    
            
            elif i.ro_left.children[0].value == 'appl':
                mor_name = get_string(i.ro_left.children[1].children[0], self.temps, self.in_temps)
                mor_arg = get_string(i.ro_left.children[1].children[1], self.temps, self.in_temps)
                mor_res = get_string(i.ro_right, self.temps, self.in_temps)
                if mor_arg not in self.appl:
                    self.appl[mor_arg] = []
                self.appl[mor_arg] += [(mor_name, mor_res)]
        
            
            
class resalt_op:
    #resalt_op(integer ID_Concept,expr,expr)// sloznij term sleva, prostoi sprava (t.e. descr)
    def __init__(self, pr):
        self.ro_conceptId = pr.tree.children[0].value
        self.ro_left = pr.tree.children[1]
        self.ro_right = pr.tree.children[2]
        
    def remake(self, temps, in_temps):
        self.ro_left = get_string(self.ro_left, temps, in_temps)
        self.ro_right = get_string(self.ro_right, temps, in_temps)

class template:
    #template(integer IdConcept, id Id_template, string Text, string Opname, string Comm, list_w_v_t ListTemplate, expr ResultType)
    def __init__(self, pred):
        self.t_text = pred.tree.children[2].value
        self.t_id = pred.tree.children[3].value
        self.t_comment = pred.tree.children[4].value
        self.t_restype = pred.tree.children[6]

class in_template:
    '''Class for in_template predicate.
    Predicate description: in_template(integer IdDostup_0_1_2, integer ID_Concept,id Id_template, string Text, string Opname, string Comment, list_w_v_t Templates_list, expr TypeRes)'''
    def __init__(self, pr):
        self.it_conceptId = pr.tree.children[1].value
        self.it_text = '_'.join(pr.tree.children[3].value.split())
        self.it_id = pr.tree.children[4].value
        self.it_comment = pr.tree.children[5].value
        self.it_restype = pr.tree.children[7]
        self.it_isMor = False
    
    def remake(self, temps, in_temps):
        self.it_restype = get_string(self.it_restype, temps, in_temps)

class mor:
    def __init__(self, name, conceptId, isFunc):
        self.name = name
        self.conceptId = conceptId
        self.dom = ''
        self.cod = ''
        self.appl = {}
        self.isFunc = isFunc
    
    def __repr__(self):
        return self.name + ' ' + str(self.isFunc) + ' ' + self.dom + ' ' + self.cod



class element:
    '''Class for element predicate.
    Predicate description: element(integer ID_Concept, expr Element, expr Ob)'''
    
    def __init__(self, pr):
        self.e_conceptId = pr.tree.children[0].value
        self.e_elem = pr.tree.children[1]
        self.e_ob = pr.tree.children[2]
        self.e_isMor = False
    
    def __repr__(self):
        return self.e_elem + ' -> ' + self.e_ob
    
    def remake(self, temps, in_temps):
        self.e_elem = get_string(self.e_elem, temps, in_temps)
        self.e_ob = get_string(self.e_ob, temps, in_temps)


class subobject:
    '''Class for subobject predicate.
    Predicate description: subobject(integer ID_Concept, expr Sub, expr Super)'''
    def __init__(self,pr):
        self.s_conceptId = pr.tree.children[0].value
        self.s_sub = pr.tree.children[1]
        self.s_super = pr.tree.children[2]
    
    def __repr__(self):
        return self.s_sub + ' -> ' + self.s_super
    
    def remake(self, temps, in_temps):
        self.s_sub = get_string(self.s_sub, temps, in_temps)
        self.s_super = get_string(self.s_super, temps, in_temps)
    

def get_string(in_ex, temps, in_temps):
    expr = in_ex.value
    if expr == 'r' or expr == 'nc':
        return in_ex.children[0].value
    elif expr == 'ex':
        sub = get_template(in_ex.children[0].value, temps, in_temps)
        res = ''
        counter = 0
        isText = True
        for i in range(0, len(sub)):
#            print counter, sub[i]
            if isText:
                if sub[i]=='@':
                    res += get_string(in_ex.children[1].children[counter], temps, in_temps)
                    counter += 1
                    isText = False
                    continue
                else:
                    res += sub[i]
            else:
                if sub[i] in ' ()':
                    res += sub[i]
                    isText = True
                else:
                    continue
        return res.replace(' ', '_')



#def get_string(in_ex, temps, in_temps):
#    return get_string_inner(in_ex, temps, in_temps).replace(' ', '_')
#    
#    
#def get_string_inner(in_ex, temps, in_temps):
#    '''For given ex expression builds string.'''
#    expr = in_ex.value
#    if expr == 'r' or expr == 'nc':
#        return in_ex.children[0].value
#    elif expr == 'ex':
#        sub = get_template(in_ex.children[0].value, temps, in_temps)
#        if len(in_ex.children[1].children)==0:
#            return sub
#        else:
#            #Problem here
#            rr = '@[а-яА-Яa-zA-Z0-9_\-]+'
#            curpos = 0
#            counter = 0
#            res =''
#            for i in re.finditer(rr, sub):
#                res += sub[curpos:i.start()]+ get_string(in_ex.children[1].children[counter], temps, in_temps)
#                curpos = i.end()
#                counter +=1
#            res += sub[curpos:]
#            return res
#    else:
#        return expr

def get_template(id, temps, in_temps):
    '''For given id returns template from kernel (first choice) or inner (second choice) template.
    If can't find any, returns id itself.'''
    if id in in_temps:
        return in_temps[id].it_text
    elif id in temps:
        return temps[id].t_text
    else:
        return id

def readfile(filename, ont_ob):
    '''Reads ontology file, fills in_temps, res_ops, elems and obs fields of given ontology object.'''
    pred_list = ['in_template', 'resalt_op', 'element', 'subobject']
    f = open(filename, mode='r')
    badluck = []
    for i in f:
        try:
            #Problem here
            if config.ENC_IN != config.ENC_OUT:
                i = i.decode(config.ENC_IN).encode(config.ENC_OUT)
            
            pr = parsePro.parse_pred_list(i, pred_list)
            if pr:
                
                if pr.name == 'resalt_op':
                    ont_ob.res_ops += [resalt_op(pr)]
                elif pr.name == 'element':
                    ont_ob.elems += [element(pr)]
                elif pr.name == 'subobject':
                    ont_ob.obs += [subobject(pr)]
                elif pr.name == 'in_template':                    
                    tmp = in_template(pr)
                    if tmp.it_id not in ont_ob.in_temps:
                        ont_ob.in_temps[tmp.it_id]=tmp
        except Exception, e:
            badluck += [i]
    f.close()
    return badluck

def readkernel(ont_ob):
    '''Reads kernel file, fills temps filed of given ontology object.'''
    pred_list = ['template']
    f = open(config.KERNEL, mode='r')
    badluck = []
    for i in f:
        try:
            if config.ENC_IN != config.ENC_OUT:
                i = i.decode(config.ENC_IN).encode(config.ENC_OUT)
            
            pr = parsePro.parse_pred_list(i, pred_list)
            if pr:
                tmp = template(pr)
                if tmp.t_id not in ont_ob.temps:
                    ont_ob.temps[tmp.t_id]=tmp
        except Exception, e:
            badluck += [i]
    f.close()
    return badluck

def check_id(id, temps):
    '''Checks if there is temp with id. If there is, returns temps[id], if not returns id.'''
    if id in temps:
            return temps[id].t_text
    else:
            return id

def check_conceptId(size, id):
    '''Checks in what mode program is. (Filter for elements and objects)'''
    return (size==config.FULL and id=='1') or id!='1'


def make_dot(ont, size):
    '''Creates dot file (returns it as line).
    Troubles with encodings, have NO IDEA what is wrong :`( '''
    
    res = 'digraph G{\nnode [fontsize=13]\n'
    
    objects = []
    superobjects = []
    
    for p in ont.obs:
        if check_conceptId(size, p.s_conceptId):
            res += '"' + p.s_sub + '"->"' + p.s_super + '";\n'
            objects += [p.s_sub]
            superobjects += [p.s_super]
    
    for e in ont.elems:
        if check_conceptId(size, e.e_conceptId):
            res += '"' + e.e_elem + '"->"' + e.e_ob + '"[style="dashed"];\n'
            objects += [e.e_elem]
            superobjects += [e.e_ob]
    
    for t in ont.in_temps:
        if check_conceptId(size, ont.in_temps[t].it_conceptId) and '@' not in ont.in_temps[t].it_text:
            #maybe i should filter templates with cl_ex, as i do with class while creating owl?
            tmp = ont.in_temps[t]
            res+= '"'+tmp.it_text + '"->"' + tmp.it_restype + '"[color=grey];\n'
            objects += [tmp.it_text]
            superobjects += [tmp.it_restype]
    
    cl_nonex = set(superobjects).difference(set(objects))
    
    for i in cl_nonex:
        res += '"'+i+'"->"'+ ont.in_temps['ob'].it_text +'"[color=grey];\n'
    
    res += '}\n'
    return res

def make_owl(ont, size):
    '''Creates xml and it's first lines. Returns document object.'''
    
    doc = minidom.Document()
    root = doc.createElement("rdf:RDF")
    doc.appendChild(root)
    root.setAttribute("xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#")
    root.setAttribute("xmlns:owl","http://www.w3.org/2002/07/owl#")
    root.setAttribute("xmlns:rdfs","http://www.w3.org/2000/01/rdf-schema#")
    
    OWLontology = doc.createElement("owl:Ontology")
    root.appendChild(OWLontology)
    OWLontology.setAttribute("rdf:about", "")
    
    RDFScomment = doc.createElement("rdfs:comment")
    OWLontology.appendChild(RDFScomment)
    RDFScomment.setAttribute("xml:lang", "ru")
    
    #translation may be needed
    
    about = doc.createTextNode('Онтология ' + ont.title + ', экспортированная из системы ЭЗОП.')
    RDFScomment.appendChild(about)
    #
    #for every found predicate
    
    objects = []
    superobjects = []

    for p in ont.obs:
        if check_conceptId(size, p.s_conceptId):
            classOwl = doc.createElement("owl:Class")
            root.appendChild(classOwl)
            classOwl.setAttribute("rdf:about", "#" + p.s_sub)
            
            subClassOfOwl = doc.createElement("rdfs:subClassOf")
            classOwl.appendChild(subClassOfOwl)
            subClassOfOwl.setAttribute("rdf:resource", "#" + p.s_super)
            
            objects += [p.s_sub]
            superobjects += [p.s_super]
    curelems = []
    for e in ont.elems:
        if check_conceptId(size, e.e_conceptId) and not(e.e_isMor) and e.e_elem not in curelems:
            owlThing = doc.createElement("owl:Thing")
            root.appendChild(owlThing)
            owlThing.setAttribute("rdf:about", "#"+e.e_elem)
            
            rdfType = doc.createElement("rdf:type")
            owlThing.appendChild(rdfType)
            rdfType.setAttribute("rdf:resource", "#"+e.e_ob)
            if e.e_elem in ont.appl:
                for property in ont.appl[e.e_elem]:
                    propname = property[0]
                    propres = property[1]
                    propOwl = doc.createElement(propname)
                    owlThing.appendChild(propOwl)
                    propOwl.setAttribute('rdf:resource', '#'+propres)
            superobjects += [e.e_ob]
            curelems += [e.e_elem]
    
    for it in ont.in_temps:
        t = ont.in_temps[it]
        
        if check_conceptId(size, t.it_conceptId) and '@' not in t.it_text and not(t.it_isMor):
            if t.it_text not in curelems:
                owlThing = doc.createElement("owl:Thing")
                root.appendChild(owlThing)
                owlThing.setAttribute("rdf:about", "#"+t.it_text)
                
                rdfType = doc.createElement("rdf:type")
                owlThing.appendChild(rdfType)
                rdfType.setAttribute("rdf:resource", "#"+t.it_restype)
                
                superobjects += [t.it_restype]    
    
    cl_nonex = set(superobjects).difference(set(objects))
    
    for p in cl_nonex:
        classOwl = doc.createElement("owl:Class")
        root.appendChild(classOwl)
        classOwl.setAttribute("rdf:about", "#" + p)
    
    for mkey in ont.mors:
        m = ont.mors[mkey]
        if check_conceptId(size, m.conceptId):
            if m.dom == '':
                m.dom = 'область_определения_'+m.name
            if m.cod == '':
                m.cod = 'область_значения_'+m.name

            owlProperty = doc.createElement('owl:ObjectProperty')
            root.appendChild(owlProperty)
            owlProperty.setAttribute('rdf:about', '#'+m.name)
            
            #if m.isFunc:
            #    rdfType = doc.createElement('rdf:type')
            #    owlProperty.appendChild(rdfType)
            #    rdfType.setAttribute('rdf:resource', '&owl;FunctionalProperty')
            
            rdfsDomain = doc.createElement('rdfs:domain')
            owlProperty.appendChild(rdfsDomain)
            rdfsDomain.setAttribute('rdf:resource', '#'+m.dom)
            
            rdfsRange = doc.createElement('rdfs:range')
            owlProperty.appendChild(rdfsRange)
            rdfsRange.setAttribute('rdf:resource', '#'+m.cod)
    
    return doc



def new_construct(fn_ont, fn_kernel, params):
    pass

def construct_debug(filename):
    #filename_in = 'C:\\xampp\\htdocs\\drupal\\EXE\\BaseExample\\10458118'
    filename_in =  'C:\\xampp\\htdocs\\drupal\\EXE\\Example\\55358100'
    type = config.OWL
    size = config.SMALL
    res = construct(filename_in, size, type)
    if res!=False:
        f = open(filename, 'w')
        f.write(res)
        f.close()
        print 'We did it!'
    else:
        print 'U a loser!!!!'


def construct(filename_in, size, type, title):
    '''Main function. Takes filename as string, size = small or full as string, type = owl or pic alse as string.
    Returns dot or xml in string..
    In case of failure returns False.'''
    
    ont = ontology(title)
    
    try:
        trash = readfile(filename_in, ont)
        trash+= readkernel(ont)
        ont.fullremake()
        ont.find_mor()
        ont.fill_mors()
        
        res = ''
        
        if type == config.PIC:
            return make_dot(ont, size)
        elif type == config.OWL:
            doc = make_owl(ont, size)
            return doc.toprettyxml()
    except:
        return False    


def foo():
    filename_in =  'C:\\xampp\\htdocs\\drupal\\EXE\\Example\\55358100'
    type = config.OWL
    size = config.FULL
    
    ont = ontology('Test')    
    
    try:
        trash = readfile(filename_in, ont)
        trash+= readkernel(ont)
        
        ont.fullremake()
        ont.find_mor()
        ont.fill_mors()
        
        return ont
        
        #res = ''
        #
        #if type == PIC:
        #    return make_dot(ont, size)
        #elif type == OWL:
        #    doc = make_owl(ont, size)
        #    return doc.toprettyxml()
    except:
        return False    
