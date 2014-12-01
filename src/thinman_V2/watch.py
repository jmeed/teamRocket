#!/usr/bin/env python2
# -*- coding: utf-8 -*-

"""
Filesystem Watcher and Automatic Command executer
@author 赵迤晨
@copyright 赵迤晨 (Zhao Yichen) <interarticle@gmail.com>
@license MIT License
@version 0.1.2

@changelog
v0.1.2: Implemented "first-matched, only-one-executed" rule, and unified path seperators to "/"
v0.1.1: Fixed incorrect CWD value when watch.yaml is in the same directory as watch.py, when executing locally.
"""
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import time
import socket
import re
from os import path
import os
import threading
import yaml
import glob
import argparse
import subprocess
import fnmatch

parser = argparse.ArgumentParser(description="""
	Filesystem changes watcher and auto command executer upon file changes that are matched using regex.
	
	This is only the client-side watcher. You also need server.py to actually execute commands
	""")
parser.add_argument('--host', '-H', dest='remoteHost', default=None)
parser.add_argument('--port', '-p', dest="remotePort", default=1238, type=int)
parser.add_argument('--update-delay', '-d', dest='update_delay', default=0.5, type=float)
parser.add_argument('watch', nargs='?', default='.', help="The directory to watch")
args = parser.parse_args()

change_delay = args.update_delay

os.chdir(args.watch)

def setTimeout(timesec, func):
	t = threading.Timer(timesec, func)
	t.start()
	return t

file_changes = {}
handler_timer = None
handlingChanges = False

watchConfigs = {}

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


def configUpdate(fn, removed = False):
	rootdir = path.dirname(fn) + path.sep
	if removed:
		if rootdir in watchConfigs:
			del watchConfigs[rootdir]
			print "Removed Configuration %s" % (fn,)
		return
	print "Loading Configuration %s" % (fn,)
	if not path.isfile(fn):
		print "Configuration %s no longer exists" % (fn,)
		return
	try:
		fconf = open(fn,'r')
		watchConfigs[rootdir] = yaml.safe_load(fconf)
		fconf.close()
	except Exception as e:
		print "Load Failed: %s" % (unicode(e),)
def handleFileChanges():
	global handler_timer, handlingChanges
	handler_timer = None
	handlingChanges = True
	handleFileChangesInternal()
	handlingChanges = False

def handleFileChangesInternal():
	global file_changes
	local_file_changes = dict(file_changes)
	file_changes.clear()
	if len(local_file_changes) == 0:
		return

	rootDirs = sorted(watchConfigs.iterkeys(), lambda x,y: cmp(len(x), len(y)), reverse=True)

	commands = {}

	for fn, status in local_file_changes.iteritems():
		if re.search(r'watch.yaml$', fn, re.I):
			configUpdate(fn, not status)
		elif status:
			for d in rootDirs:
				if fn.find(d) == 0:
					fn = fn[len(d):].strip(path.sep)
					fn = fn.replace(path.sep, '/')
					if not d in commands:
						commands[d] = {}
					for instruction in watchConfigs[d]:
						if re.match(instruction['match'], fn, re.I):
							print fn
							commands[d][instruction['command']] = re.match(instruction['match'], fn, re.I)
							break
					break

	for d in commands.iterkeys():
		for cmd in commands[d].iterkeys():
			if not cmd:
				continue
			try:
				exeDir = d[(len(path.abspath('.')) + 1):]
				cmd = re.sub(r'\$(\d+)', lambda m: commands[d][cmd].group(int(m.group(1))), cmd)
				print 'Execute: ', exeDir, cmd
				if args.remoteHost:
					sock.sendto('%s\n%s'% (exeDir.replace(path.sep,'/'), cmd), (args.remoteHost, args.remotePort))
				else:
					subprocess.Popen(cmd, cwd=path.abspath(exeDir), shell=True).wait()
			except Exception as e:
				print "Command Execution Failed for %s: %s" % (cmd, unicode(e),)
	file_changes.clear()

def escapeShellString(fn):
	if os.name == 'nt':
		return '"' + fn.replace('"', '""') + '"'
	else:
		return "'" + fn.replace("'", "'\\''") + "'"

def queueFileHandling():
	global handler_timer
	if handler_timer:
		handler_timer.cancel()
	if not handlingChanges:
		#Fail silently if currently handling changes
		handler_timer = setTimeout(change_delay, handleFileChanges)

class handler(FileSystemEventHandler):
	def on_any_event(self, event):
		if type(event).__name__ == 'FileModifiedEvent' or type(event).__name__ == 'FileCreatedEvent':
			file_changes[event.src_path] = True
		elif type(event).__name__ == 'FileMovedEvent':
			file_changes[event.src_path] = False
			file_changes[event.dest_path] = True
		elif type(event).__name__ == 'FileDeletedEvent':
			file_changes[event.src_path] = False
		queueFileHandling()

for root, dirnames, filenames in os.walk('.'):
  for filename in fnmatch.filter(filenames, 'watch.yaml'):
      file_changes[path.abspath(os.path.join(root, filename))] = True
handleFileChanges()

observer = Observer()
observer.schedule(handler(),'.',recursive=True)
observer.start()

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    observer.stop()
except OSError:
	print "Shit"
	raise
observer.join()
