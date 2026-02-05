const { spawnSync } = require("child_process");
const path = require("path");

// Use require.resolve to find node-pre-gyp, which works with all package managers
// including pnpm which uses a different node_modules structure
const nodePreGypDir = path.dirname(require.resolve("@mapbox/node-pre-gyp/package.json"));
const nodePreGyp = path.join(nodePreGypDir, "bin", "node-pre-gyp");

const args = ["install", "--fallback-to-build"];

if (process.env.DUCKDB_NODE_BINARY_HOST) {
  args.push("--host", process.env.DUCKDB_NODE_BINARY_HOST);
}

const result = spawnSync(process.execPath, [nodePreGyp, ...args], {
  stdio: "inherit",
});

process.exit(result.status ?? 1);
