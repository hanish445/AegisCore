const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const fs = require('fs');
const path = require('path');
const { parseTrace } = require('./parser');

const app = express();
const server = http.createServer(app);
const io = new Server(server);

const PORT = 3000;
const LOGS_DIR = path.join(__dirname, '..', 'build', 'logs');

// Serve frontend files from the 'public' directory
app.use(express.static(path.join(__dirname, 'public')));

// When a browser connects, send the most recent crash trace immediately
io.on('connection', (socket) => {
    console.log('Frontend dashboard connected.');
    broadcastLatestTrace();
});

function broadcastLatestTrace() {
    if (!fs.existsSync(LOGS_DIR)) return;
    
    // Find the newest JSON file
    const files = fs.readdirSync(LOGS_DIR)
        .filter(fn => fn.endsWith('.json'))
        .map(fn => ({ name: fn, time: fs.statSync(path.join(LOGS_DIR, fn)).mtime.getTime() }))
        .sort((a, b) => b.time - a.time);

    if (files.length > 0) {
        const latestFile = path.join(LOGS_DIR, files[0].name);
        const traceData = parseTrace(latestFile);
        if (traceData) {
            io.emit('new_trace', traceData);
        }
    }
}

// Watch the logs directory for newly generated traces from Python/C++
if (!fs.existsSync(LOGS_DIR)) {
    fs.mkdirSync(LOGS_DIR, { recursive: true });
}

fs.watch(LOGS_DIR, (eventType, filename) => {
    if (filename && filename.endsWith('.json')) {
        // Slight delay ensures C++ and Python are fully finished writing the file
        setTimeout(broadcastLatestTrace, 100);
    }
});

server.listen(PORT, () => {
    console.log(`Threat Reporting Dashboard running at http://localhost:${PORT}`);
});