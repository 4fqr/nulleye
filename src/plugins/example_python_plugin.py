# example Python plugin for NullEye

def init(core):
    core.log_info('example_python_plugin initialized')

def fini(core):
    core.log_info('example_python_plugin shutdown')

def process_event(core, event):
    if event.get('module') == 'ebpf' and event.get('comm') == 'suspicious':
        core.alert('python-plugin', 'suspicious process')
