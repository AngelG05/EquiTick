import React, { useState, useEffect } from 'react';
import { Line, Bar } from 'react-chartjs-2';
import { 
  Activity, 
  ShieldCheck, 
  Zap, 
  AlertTriangle, 
  Clock, 
  CheckCircle2 
} from 'lucide-react';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  BarElement,
  Title,
  Tooltip,
  Legend,
  Filler
} from 'chart.js';
import { motion } from 'framer-motion';

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  BarElement,
  Title,
  Tooltip,
  Legend,
  Filler
);

const Dashboard = () => {
  const [metrics, setMetrics] = useState({
    throughput: 0,
    latency: 0,
    blocked: 0,
    bias: 0.12,
    events: 0
  });

  const [history, setHistory] = useState({
    labels: Array(20).fill(''),
    latency: Array(20).fill(0),
    bias: Array(20).fill(0)
  });

  // Simulated real-time updates for demonstration
  useEffect(() => {
    const interval = setInterval(() => {
      setMetrics(prev => ({
        throughput: 524000 + Math.random() * 50000,
        latency: 720 + Math.random() * 80,
        blocked: prev.blocked + (Math.random() > 0.95 ? 1 : 0),
        bias: 0.12 + Math.random() * 0.05,
        events: prev.events + 1000
      }));

      setHistory(prev => {
        const newLatency = [...prev.latency.slice(1), 720 + Math.random() * 80];
        const newBias = [...prev.bias.slice(1), 0.12 + Math.random() * 0.05];
        return {
          labels: prev.labels,
          latency: newLatency,
          bias: newBias
        };
      });
    }, 500);
    return () => clearInterval(interval);
  }, []);

  const latencyData = {
    labels: history.labels,
    datasets: [{
      label: 'Latency (µs)',
      data: history.latency,
      borderColor: '#3b82f6',
      backgroundColor: 'rgba(59, 130, 246, 0.1)',
      fill: true,
      tension: 0.4,
      pointRadius: 0
    }]
  };

  const biasData = {
    labels: history.labels,
    datasets: [{
      label: 'Bias Coefficient',
      data: history.bias,
      borderColor: '#10b981',
      backgroundColor: 'rgba(16, 185, 129, 0.1)',
      fill: true,
      tension: 0.4,
      pointRadius: 0
    }]
  };

  const chartOptions = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: { legend: { display: false } },
    scales: {
      x: { display: false },
      y: { grid: { color: 'rgba(255,255,255,0.05)' } }
    }
  };

  return (
    <div className="p-8 max-w-7xl mx-auto h-screen flex flex-col gap-6">
      <header className="flex justify-between items-center">
        <div>
          <h1 className="text-3xl font-bold bg-gradient-to-r from-blue-400 to-emerald-400 bg-clip-text text-transparent">
            EquiTick
          </h1>
          <p className="text-gray-400 font-mono text-sm">Real-time Pay Equity Enforcement Engine (v2.0-Elite)</p>
        </div>
        <div className="flex items-center gap-4">
          <div className="flex items-center gap-2 text-emerald-400 glass px-3 py-1 text-sm">
            <div className="w-2 h-2 rounded-full bg-emerald-400 animate-pulse"></div>
            SYSTEM OPERATIONAL
          </div>
          <div className="glass px-3 py-1 text-sm font-mono">{new Date().toLocaleTimeString()}</div>
        </div>
      </header>

      {/* Stats Grid */}
      <div className="grid grid-cols-4 gap-6">
        <StatCard icon={<Zap size={20} className="text-blue-400" />} label="Throughput" value={`${(metrics.throughput / 1000).toFixed(1)}k`} unit="evt/s" />
        <StatCard icon={<Clock size={20} className="text-emerald-400" />} label="E2E Latency" value={metrics.latency.toFixed(0)} unit="µs" />
        <StatCard icon={<AlertTriangle size={20} className="text-danger" />} label="Blocked Decisions" value={metrics.blocked} unit="total" />
        <StatCard icon={<ShieldCheck size={20} className="text-purple-400" />} label="Bias Variance" value={(metrics.bias * 100).toFixed(2)} unit="%" />
      </div>

      {/* Main Charts */}
      <div className="grid grid-cols-2 gap-6 flex-1 min-h-0">
        <div className="glass p-6 flex flex-col">
          <h2 className="text-lg font-semibold mb-4 flex items-center gap-2">
            <Activity size={18} /> Latency Distribution
          </h2>
          <div className="flex-1">
            <Line data={latencyData} options={chartOptions} />
          </div>
        </div>
        <div className="glass p-6 flex flex-col">
          <h2 className="text-lg font-semibold mb-4 flex items-center gap-2">
            <CheckCircle2 size={18} /> Bias Regression Drift
          </h2>
          <div className="flex-1">
            <Line data={biasData} options={chartOptions} />
          </div>
        </div>
      </div>

      {/* Bottom Ticker */}
      <footer className="glass p-4 font-mono text-xs text-gray-500 overflow-hidden relative">
        <div className="flex gap-8 whitespace-nowrap animate-marquee">
          <span>[SYSTEM] CPU: 12% | MEM: 4.2GB | NUMA_NODE: 0 | PGO_ENABLED: TRUE</span>
          <span>[ENGINE] THREAD_PINNED: CORE_2 | SIMD_BATCH_SIZE: 128 | LOCK_FREE: OK</span>
          <span>[NETWORK] FEED_STREAMS: 4 | ZERO_COPY_PARSING: ENABLED | PACKET_DROP: 0.00%</span>
        </div>
      </footer>
    </div>
  );
};

const StatCard = ({ icon, label, value, unit }) => (
  <motion.div 
    whileHover={{ y: -5 }}
    className="glass p-5 flex items-center justify-between"
  >
    <div>
      <p className="text-gray-400 text-sm font-medium mb-1">{label}</p>
      <div className="flex items-baseline gap-2">
        <span className="text-2xl font-bold font-mono">{value}</span>
        <span className="text-xs text-gray-500 uppercase">{unit}</span>
      </div>
    </div>
    <div className="bg-white/5 p-3 rounded-lg">
      {icon}
    </div>
  </motion.div>
);

export default Dashboard;
