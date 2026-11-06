const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = 3000;

const server = http.createServer((req, res) => {
    if(req.url == '/'){
        res.writeHead(200, { 'Content-Type': 'text/html' });
        res.end('<h1>AegisCore Threat Report Dashbpard</h1><p>Waiting for DTA logs...</p>')
    }
    else{
        res.writeHead(404);
        res.end('Not found');
    }
});

server.listen(PORT, () => {
    console.log(`[Reporter] Dashboard running at http://localhost:${PORT}`);
});