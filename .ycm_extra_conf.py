def FlagsForFile(filename, **kwargs):
    return {
        'flags': ['-std=gnu99', '-Wall', '-lm', '-lfl', '-lreadline', '-I', 'include'],
    }
