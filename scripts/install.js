const { spawnSync } = require("child_process");
const path = require("path");

const nodePreGyp = path.join(
  __dirname,
  "..",
  "node_modules",
  "@mapbox",
  "node-pre-gyp",
  "bin",
  "node-pre-gyp"
);

const args = ["install", "--fallback-to-build"];

if (process.env.DUCKDB_NODE_BINARY_HOST) {
  args.push("--host", process.env.DUCKDB_NODE_BINARY_HOST);
}

const result = spawnSync(process.execPath, [nodePreGyp, ...args], {
  stdio: "inherit",
});

process.exit(result.status ?? 1);
