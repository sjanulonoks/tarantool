
box.cfg.sophia
box.cfg.sophia.threads = 5

space = box.schema.space.create('test', { engine = 'sophia' })
index = space:create_index('primary', {type = 'tree', amqf=1, read_oldest=0, parts = {1, 'NUM'}})
box.sophia["db.".. tostring(space.id)..".amqf"]
box.sophia["db.".. tostring(space.id)..".mmap"]
space:drop()