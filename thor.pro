TEMPLATE = subdirs

SUBDIRS += \
    server \
    sample

server.subdir = server
sample.subdir = sample

sample.depends = server
