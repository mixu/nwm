var repl = require('repl')
var net = require('net')

exports.createServer = function (context) {
	return  net.createServer(function (socket) {
		var r = repl.start({
				prompt: '>'
			, input: socket
			, output: socket
			, terminal: true
      , ignoreUndefined: true
			, useGlobal: false
		})
		r.on('exit', function () {
			socket.end()
		})

    if(context)
      for (var k in context) {
        r.context[k] = (
            'function' == typeof context[k]
          ? context[k].bind(context)
          : context[k]
        )
      }
	})
}

exports.connect = function () {
  var args = [].slice.call(arguments)
  var sock = net.connect.apply(net, args)

  process.stdin.pipe(sock)
  sock.pipe(process.stdout)

  sock.on('connect', function () {
    process.stdin.resume();
    process.stdin.setRawMode(true)
  })

  sock.on('close', function done () {
    process.stdin.setRawMode(false)
    process.stdin.pause()
    sock.removeListener('close', done)
  })

  process.stdin.on('end', function () {
    sock.destroy()
    console.log()
  })

  process.stdin.on('data', function (b) {
    if (b.length === 1 && b[0] === 4) {
      process.stdin.emit('end')
    }
  })

  return sock
}

//for testing.
if(!module.parent) {
  var args = process.argv.slice(2)
  var type = args.shift()
  var port = args.shift() || 1337
  if('server' == type)
    exports.createServer(global).listen(port)
  else
    exports.connect(port)
}
