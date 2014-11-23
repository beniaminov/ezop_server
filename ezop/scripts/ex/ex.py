#!C:\python27\python.exe
# coding: utf-8

import sys, cgi, subprocess, os
from time import time
import fileConstruct
import config

def filler(s):
    print "Content-type:text/html\r\n\r\n"
    print "<html><head/><body>"+s+"</body></html>"

def p(s):
    return '<p>' + s + '</p>'

def img(s):
    return "<img src=" + s +"></img>"

def page(title, s):
    print "Content-type:text/html\r\n\r\n"
    print '<html><head><meta http-equiv="Content-Type" content="text/html; charset=utf-8" /><title>Схема онтологии "' + title + '"</title></head><body>'+s+'</body></html>'

def get_full_path(ont):
    for dir in config.EXPATH:
        fname = os.path.join(dir, ont)
        if os.path.isfile(fname):
            return dir+ont
    return ''

def make_picture(str_result):
    a = str(os.getpid())+ '.' + str(time())
    f_dot ="tmp/" + a +".dot"
    f_png ="tmp/" + a +".png"
    try:
        f=open(f_dot,"w")
        f.write(str_result)
        f.close()
        subprocess.call(["dot","-T", "png", "-o", f_png, f_dot])
        
        return f_png, True
    except Exception, e:
        return e, False


def give_picture(ont, str_result):
    result, succeed = make_picture(str_result)
    if succeed:
        page(ont, img(config.LEGEND)+"<br/>"+img(result))
    else:
        filler(e)

def give_owl(ont, str_result):
    ont_title='_'.join(ont.split())
    print 'Content-Type:application/owl+xml; charset=UTF-8'
    print 'Content-disposition: attachment; filename='+ont_title+'.owl'
    print
    print str_result

def main():
    form = cgi.FieldStorage()
    #    return
    
    clear_tmp()
    
    try:
        size = form.getvalue('size')
        type = form.getvalue('type')
        curcncpt_id = form.getvalue('curcnpt_id').strip()
        curcncpt_title = form.getvalue('curcnpt_title').strip()
    except Exception, err:
        filler(err)
        
    in_f = get_full_path(curcncpt_id)
    
    if in_f == '':
        filler('No such ontology.')
        return
    
    str_result = fileConstruct.construct(in_f, size, type, curcncpt_title)
    if not(str_result):
        filler('Construction failure.')
        return
    
    if type==config.PIC:
        give_picture(curcncpt_title, str_result)
    elif type==config.OWL:
        #give_owl(curcncpt_title, str_result)
        give_owl(curcncpt_id, str_result)
    else:
        filler(p('Bad type'))

def clear_tmp():
    files = os.listdir(config.TMPDIR)
    now = time()
    for i in files:
        try:
            itime = os.path.getctime(config.TMPDIR+i)
            if now-itime > config.CLEAR_PERIOD:
                os.remove(config.TMPDIR+i)
        except Exception, e:
            pass

    
def debug(size='small', type='pic', ontology='41846610'):
    '''Returns nothing, does anything you want.
    All result files is put in TMPDIR'''
    
    clear_tmp()
    title = 'TEST'
    
    in_f = get_full_path(ontology)
    
    str_result = fileConstruct.construct(in_f, size, type, title)
    
    if not(str_result):
        print 'Construction failure.'
        return
    
    if type == config.PIC:
        make_picture(str_result)
        print 'I\'ve painted a picture for you! It\'s in ' + config.TMPDIR
    elif type == config.OWL:
        fres = os.path.join(config.TMPDIR, ontology+'.xml')
        f = open(fres, 'w')
        f.write(str_result)
        f.close()
        print 'Owl file was constructed.'
    else:
        print 'You are loser, baby.'
    
    
if __name__ == "__main__":
    main()