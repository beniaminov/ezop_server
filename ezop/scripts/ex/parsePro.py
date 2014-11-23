#coding: utf-8
import re

reint = "([0-9]+)"
restr = '(?:"(.*?)(?<!\\\\)")'
rereal = "([0-9]+(?:\.[0-9]+)?)"


def reg_list(entity):
	return '(?:\[\])|' + '\[((?:'+entity+',)*'+entity+')\]'

reint_list = reg_list(reint)
restr_list = reg_list(restr)
rereal_list = reg_list(rereal)

t_types = (rereal, reint, restr)
t_lists = (rereal_list, reint_list, restr_list) 
corr = {rereal_list:rereal, reint_list:reint, restr_list:restr}

#Maybe i should use tuples instead of lists???

class node:
	'''Node class (actually kind of tree). Value as value of this node (contant or name of predicate).
	Children as children trees.'''
	def __init__(self, val, *ch):
		self.value = val
		self.children = list(ch)

	def __repr__(self):
		return self.inner_out(0)
	
	'''For this node returns it's representation as string. '''
	def out(self):
		return self.inner_out(0)
	
	'''Inner function for representation.'''
	def inner_out(self, n):
		s = '\t'*n + self.value + '\n'
		for i in self.children:
			s += i.inner_out(n+1)
		return s

class predicate:
	'''Contains predicate's name and it's constructed tree.'''
	def __init__(self, name, tree):
		self.name = name
		self.tree = tree

def inb_type(s, arr):
	'''Determs type of variable; arr - t_types or t_lists (integer or integer_list etc)'''
	for t in arr:
		k = re.match(t,s)
		if k:
			return k
	return False

def inb_list_add(tree, s, k):
	'''Add content of list to tree (as leafs).'''
	new_node = node('LIST')
	tree.children += [new_node]
	reg_type = corr[k.re.pattern]
	for i in re.findall(reg_type,k.group()):
		new_node.children += [node(i)]
	s = s[k.end():]
	return make_tree(tree,s)

def inb_type_add(tree, s, k):
	'''Add one variable as leaf. s is string to parse, k - pattern(?)'''
	tree.children += [node(k.groups()[0])]
	s = s[k.end():]
	return make_tree(tree,s)


def parse_pred(s):
	'''Was the main function - starts parsing, creates node with value equals to name of predicate.'''
	num = s.find('(')
	pred_name = s[:num]
	new_tree = node(pred_name)
	make_tree(new_tree,s[num+1:])
	pred = predicate(pred_name, new_tree)
	return pred

def make_tree(tree, s):
	'''Makes term's tree. Recursive.'''
	#preparation for parsing
	if s == '':
		return s
	elif s[0] == ')' or s[0]==']':
		return s[1:]
	elif s[0] == ',':
		s = s[1:]
	#if there is a simple variable in the beginnig of the string - processing and stop
	k = inb_type(s,t_types)
	if k:
		return inb_type_add(tree,s,k)
	#if there is a list of variables in the beginnig of the string - processing and stop
	k = inb_type(s,t_lists)
	if k:
		return inb_list_add(tree,s,k)
	#if it is not a variable or a list of variables, then it should be a functor (or list of them)
	if s[0] == '[':
		new_node = node('LIST')
		num = 0
	else:
		num = s.find('(')
		new_node = node(s[:num])
	tree.children += [new_node]
	s = make_tree(new_node, s[num+1:])
	return make_tree(tree, s)

def parse_pred_list(s, pr_list):
	'''The second main function. Makes trees only for predicates with names from str_list. Returns predicate.'''
	num = s.find('(')
	pred_name = s[:num]
	if pred_name not in pr_list:
		return None
	else:
		new_tree = node(pred_name)
		make_tree(new_tree,s[num+1:])
		pred = predicate(pred_name, new_tree)
		return pred