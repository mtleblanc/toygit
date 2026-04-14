vim.opt.makeprg = "cmake --build build/debug"

local dap = require("dap")

dap.configurations.cpp = {
  {
    name = "Launch",
    type = "codelldb",
    request = "launch",
    program = function()
      local bins = vim.fn.glob(vim.fn.getcwd() .. "/build/debug/bin/*", false, true)
      if #bins == 1 then
        return bins[1]
      end
      return vim.fn.input("Executable: ", vim.fn.getcwd() .. "/build/debug/bin/", "file")
    end,
    cwd = "${workspaceFolder}",
    stopOnEntry = false,
    args = {},
  },
}
