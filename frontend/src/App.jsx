import { useState, useRef } from "react"
import { ThemeProvider } from "@/components/theme-provider"
import { Button } from "@/components/ui/button"
import { ModeToggle } from "@/components/mode-toggle"

const SAMPLE_PUZZLE = [
  [5, 3, 0, 0, 7, 0, 0, 0, 0],
  [6, 0, 0, 1, 9, 5, 0, 0, 0],
  [0, 9, 8, 0, 0, 0, 0, 6, 0],
  [8, 0, 0, 0, 6, 0, 0, 0, 3],
  [4, 0, 0, 8, 0, 3, 0, 0, 1],
  [7, 0, 0, 0, 2, 0, 0, 0, 6],
  [0, 6, 0, 0, 0, 0, 2, 8, 0],
  [0, 0, 0, 4, 1, 9, 0, 0, 5],
  [0, 0, 0, 0, 8, 0, 0, 7, 9],
]

function createEmptyGrid() {
  return Array(9)
    .fill(null)
    .map(() => Array(9).fill(0))
}

function createEmptyBooleanGrid() {
  return Array(9)
    .fill(null)
    .map(() => Array(9).fill(false))
}

function App() {
  const [grid, setGrid] = useState(createEmptyGrid)
  const [initialCells, setInitialCells] = useState(createEmptyBooleanGrid)
  const [status, setStatus] = useState(null)
  const [loading, setLoading] = useState(false)
  const cellRefs = useRef({})

  const handleCellChange = (r, c, val) => {
    let num = 0
    if (val !== "") {
      const parsed = parseInt(val.slice(-1), 10)
      if (isNaN(parsed) || parsed < 1 || parsed > 9) return
      num = parsed
    }

    const nextGrid = grid.map((rowArr, rowIdx) =>
      rowArr.map((cellVal, colIdx) => (rowIdx === r && colIdx === c ? num : cellVal))
    )
    setGrid(nextGrid)

    const nextInitial = initialCells.map((rowArr, rowIdx) =>
      rowArr.map((isInit, colIdx) => (rowIdx === r && colIdx === c ? num > 0 : isInit))
    )
    setInitialCells(nextInitial)
    setStatus(null)
  }

  const handleKeyDown = (e, r, c) => {
    if (e.key === "Backspace" || e.key === "Delete") {
      handleCellChange(r, c, "")
      return
    }

    let targetR = r
    let targetC = c

    if (e.key === "ArrowUp") targetR = Math.max(0, r - 1)
    else if (e.key === "ArrowDown") targetR = Math.min(8, r + 1)
    else if (e.key === "ArrowLeft") targetC = Math.max(0, c - 1)
    else if (e.key === "ArrowRight") targetC = Math.min(8, c + 1)
    else return

    e.preventDefault()
    const targetKey = `${targetR}-${targetC}`
    if (cellRefs.current[targetKey]) {
      cellRefs.current[targetKey].focus()
      cellRefs.current[targetKey].select()
    }
  }

  const handleSolve = async () => {
    setLoading(true)
    setStatus(null)

    // Mark current numbers as initial clues
    const currentInitials = grid.map((row) => row.map((cell) => cell > 0))
    setInitialCells(currentInitials)

    try {
      const res = await fetch("/api/solve", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ grid }),
      })

      if (!res.ok) {
        const data = await res.json().catch(() => ({}))
        setStatus({
          type: "error",
          message: data.error || `Server error (${res.status})`,
        })
        return
      }

      const data = await res.json()
      if (data.success && data.solution) {
        setGrid(data.solution)
        setStatus({
          type: "success",
          message: "Puzzle solved successfully.",
        })
      } else {
        setStatus({
          type: "error",
          message: data.error || "The puzzle cannot be solved.",
        })
      }
    } catch {
      setStatus({
        type: "error",
        message: "Could not connect to backend server. Ensure it is running on port 5000.",
      })
    } finally {
      setLoading(false)
    }
  }

  const handleClear = () => {
    setGrid(createEmptyGrid())
    setInitialCells(createEmptyBooleanGrid())
    setStatus(null)
  }

  const handleLoadSample = () => {
    setGrid(SAMPLE_PUZZLE.map((row) => [...row]))
    setInitialCells(SAMPLE_PUZZLE.map((row) => row.map((val) => val > 0)))
    setStatus({
      type: "info",
      message: "Sample puzzle loaded.",
    })
  }

  return (
    <ThemeProvider defaultTheme="dark" storageKey="vite-ui-theme">
      <div className="flex flex-col items-center justify-center min-h-screen px-4 py-8 bg-background text-foreground transition-colors">
        <header className="flex items-center justify-between w-full max-w-md mb-6">
          <div>
            <h1 className="text-2xl font-bold tracking-tight">Sudoku Solver</h1>
            <p className="text-xs text-muted-foreground mt-0.5">
              Constraint satisfaction and backtracking solver
            </p>
          </div>
          <ModeToggle />
        </header>

        <main className="flex flex-col items-center w-full max-w-md">
          {/* 9x9 Sudoku Board */}
          <div className="grid grid-cols-9 border-2 border-foreground rounded-md overflow-hidden bg-card shadow-sm">
            {grid.map((row, r) =>
              row.map((cellValue, c) => {
                const borderBottom =
                  r === 2 || r === 5
                    ? "border-b-2 border-b-foreground"
                    : r === 8
                    ? ""
                    : "border-b border-b-border"

                const borderRight =
                  c === 2 || c === 5
                    ? "border-r-2 border-r-foreground"
                    : c === 8
                    ? ""
                    : "border-r border-r-border"

                const isInitial = initialCells[r][c]
                const isSolved = cellValue > 0 && !isInitial

                return (
                  <input
                    key={`${r}-${c}`}
                    ref={(el) => (cellRefs.current[`${r}-${c}`] = el)}
                    type="text"
                    inputMode="numeric"
                    maxLength={1}
                    value={cellValue === 0 ? "" : cellValue}
                    onChange={(e) => handleCellChange(r, c, e.target.value)}
                    onKeyDown={(e) => handleKeyDown(e, r, c)}
                    onFocus={(e) => e.target.select()}
                    className={`w-9 h-9 sm:w-11 sm:h-11 text-center text-lg sm:text-xl outline-none transition-colors bg-transparent
                      ${borderBottom} ${borderRight}
                      ${isInitial ? "font-bold text-foreground" : ""}
                      ${isSolved ? "font-semibold text-blue-600 dark:text-blue-400" : ""}
                      focus:bg-accent/40`}
                  />
                )
              })
            )}
          </div>

          {/* Action buttons */}
          <div className="flex flex-wrap items-center justify-center gap-2 mt-6 w-full">
            <Button
              onClick={handleSolve}
              disabled={loading}
              className="flex-1 min-w-[100px]"
            >
              {loading ? "Solving..." : "Solve"}
            </Button>
            <Button
              variant="outline"
              onClick={handleLoadSample}
              disabled={loading}
              className="flex-1 min-w-[100px]"
            >
              Load Sample
            </Button>
            <Button
              variant="secondary"
              onClick={handleClear}
              disabled={loading}
              className="flex-1 min-w-[100px]"
            >
              Clear
            </Button>
          </div>

          {/* Status feedback */}
          {status && (
            <div
              className={`mt-4 px-4 py-2 text-sm rounded-md w-full text-center border ${
                status.type === "success"
                  ? "bg-green-500/10 text-green-700 dark:text-green-400 border-green-500/20"
                  : status.type === "error"
                  ? "bg-destructive/10 text-destructive border-destructive/20"
                  : "bg-muted text-muted-foreground border-border"
              }`}
            >
              {status.message}
            </div>
          )}
        </main>
      </div>
    </ThemeProvider>
  )
}

export default App
