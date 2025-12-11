import React, { useState, useEffect, useMemo, useRef } from 'react';
import { BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer, PieChart, Pie, Cell } from 'recharts';
import { User, MediaItem, Role, ChatMessage, Livre, Video, Audio, Ebook, AudioBook } from './types';
import { Button, Input, Select, Card, Badge, Modal } from './components/UI';
import { askLibrarian } from './services/geminiService';

// --- Helper Functions ---
const hashPassword = (pass: string): string => {
  // Simple hash for demo parity with C++ example. In prod use bcrypt.
  let hash = 0;
  for (let i = 0; i < pass.length; i++) {
    const char = pass.charCodeAt(i);
    hash = ((hash << 5) - hash) + char;
    hash = hash & hash; // Convert to 32bit integer
  }
  return hash.toString(16);
};

// --- Initial Data ---
const INITIAL_USERS: User[] = [
  { username: 'client', passwordHash: hashPassword('123'), role: 'Client' },
  { username: 'admin', passwordHash: hashPassword('456'), role: 'Admin' },
  { username: 'superadmin', passwordHash: hashPassword('789'), role: 'SuperAdmin' },
];

const INITIAL_MEDIA: MediaItem[] = [
  { id: 1, titre: 'The Great Gatsby', dispo: true, type: 'Livre', auteur: 'F. Scott Fitzgerald', nPage: 218 },
  { id: 2, titre: 'Inception', dispo: true, type: 'Video', duree: 148, qualite: '4K' },
  { id: 3, titre: 'Dark Side of the Moon', dispo: false, type: 'Audio', publicateur: 'Pink Floyd', duree: 43 },
  { id: 4, titre: 'Clean Code', dispo: true, type: 'Ebook', auteur: 'Robert C. Martin', nPage: 464, tailleMo: 12.5, format: 'PDF' },
  { id: 5, titre: 'Becoming', dispo: true, type: 'AudioBook', auteur: 'Michelle Obama', nPage: 0, publicateur: 'Michelle Obama', duree: 1140 },
];

// --- Main Component ---
export default function App() {
  // State
  const [currentUser, setCurrentUser] = useState<User | null>(null);
  const [users, setUsers] = useState<User[]>(INITIAL_USERS);
  const [media, setMedia] = useState<MediaItem[]>(INITIAL_MEDIA);
  const [activeTab, setActiveTab] = useState<'catalog' | 'stats' | 'users' | 'ai'>('catalog');
  
  // Persistence Loading
  useEffect(() => {
    const savedUsers = localStorage.getItem('users');
    const savedMedia = localStorage.getItem('media');
    if (savedUsers) setUsers(JSON.parse(savedUsers));
    if (savedMedia) setMedia(JSON.parse(savedMedia));
  }, []);

  // Persistence Saving
  useEffect(() => {
    localStorage.setItem('users', JSON.stringify(users));
  }, [users]);

  useEffect(() => {
    localStorage.setItem('media', JSON.stringify(media));
  }, [media]);

  // Handlers
  const handleLogin = (user: User) => {
    setCurrentUser(user);
    setActiveTab('catalog');
  };

  const handleLogout = () => {
    setCurrentUser(null);
  };

  if (!currentUser) {
    return <AuthScreen users={users} onLogin={handleLogin} onRegister={(u) => setUsers([...users, u])} />;
  }

  return (
    <div className="min-h-screen bg-slate-50 flex flex-col font-sans">
      {/* Navbar */}
      <header className="bg-white border-b border-slate-200 sticky top-0 z-40">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex justify-between h-16">
            <div className="flex items-center gap-4">
              <div className="flex-shrink-0 flex items-center gap-2">
                <div className="w-8 h-8 bg-indigo-600 rounded-lg flex items-center justify-center">
                  <span className="text-white font-bold text-lg">L</span>
                </div>
                <span className="text-xl font-bold text-slate-900 hidden sm:block">LibraSys</span>
              </div>
              <nav className="hidden md:flex ml-8 gap-1">
                <NavButton label="Catalog" active={activeTab === 'catalog'} onClick={() => setActiveTab('catalog')} />
                {(currentUser.role === 'Admin' || currentUser.role === 'SuperAdmin') && (
                  <NavButton label="Statistics" active={activeTab === 'stats'} onClick={() => setActiveTab('stats')} />
                )}
                {currentUser.role === 'SuperAdmin' && (
                  <NavButton label="Users" active={activeTab === 'users'} onClick={() => setActiveTab('users')} />
                )}
                 <NavButton label="AI Librarian" active={activeTab === 'ai'} onClick={() => setActiveTab('ai')} icon={
                   <svg className="w-4 h-4 mr-1.5" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M19.428 15.428a2 2 0 00-1.022-.547l-2.384-.477a6 6 0 00-3.86.517l-.318.158a6 6 0 01-3.86.517L6.05 15.21a2 2 0 00-1.806.547M8 4h8l-1 1v5.172a2 2 0 00.586 1.414l5 5c1.26 1.26.367 3.414-1.415 3.414H4.828c-1.782 0-2.674-2.154-1.414-3.414l5-5A2 2 0 009 10.172V5L8 4z" /></svg>
                 } />
              </nav>
            </div>
            <div className="flex items-center gap-4">
              <div className="text-right hidden sm:block">
                <p className="text-sm font-medium text-slate-900">{currentUser.username}</p>
                <p className="text-xs text-slate-500">{currentUser.role}</p>
              </div>
              <Button variant="ghost" size="sm" onClick={handleLogout}>Log out</Button>
            </div>
          </div>
        </div>
        {/* Mobile Nav */}
        <div className="md:hidden border-t border-slate-100 p-2 flex gap-2 overflow-x-auto">
             <NavButton label="Catalog" active={activeTab === 'catalog'} onClick={() => setActiveTab('catalog')} size="sm"/>
             {(currentUser.role === 'Admin' || currentUser.role === 'SuperAdmin') && (
                <NavButton label="Stats" active={activeTab === 'stats'} onClick={() => setActiveTab('stats')} size="sm"/>
             )}
             {currentUser.role === 'SuperAdmin' && (
                <NavButton label="Users" active={activeTab === 'users'} onClick={() => setActiveTab('users')} size="sm"/>
             )}
              <NavButton label="AI Bot" active={activeTab === 'ai'} onClick={() => setActiveTab('ai')} size="sm"/>
        </div>
      </header>

      {/* Main Content */}
      <main className="flex-1 max-w-7xl w-full mx-auto px-4 sm:px-6 lg:px-8 py-8">
        {activeTab === 'catalog' && (
          <CatalogView 
            media={media} 
            userRole={currentUser.role} 
            onUpdate={(updated) => setMedia(media.map(m => m.id === updated.id ? updated : m))}
            onDelete={(id) => setMedia(media.filter(m => m.id !== id))}
            onAdd={(item) => setMedia([...media, item])}
          />
        )}
        {activeTab === 'stats' && <StatsView media={media} />}
        {activeTab === 'users' && (
          <UserManagementView 
            users={users} 
            currentUsername={currentUser.username}
            onUpdateUser={(updated) => setUsers(users.map(u => u.username === updated.username ? updated : u))}
            onDeleteUser={(username) => setUsers(users.filter(u => u.username !== username))}
            onAddUser={(newUser) => setUsers([...users, newUser])}
          />
        )}
        {activeTab === 'ai' && <AILibrarianView media={media} />}
      </main>
    </div>
  );
}

// --- Sub Components ---

const NavButton = ({ label, active, onClick, icon, size = 'md' }: any) => (
  <button
    onClick={onClick}
    className={`
      inline-flex items-center px-3 py-2 rounded-md text-sm font-medium transition-colors
      ${active ? 'bg-indigo-50 text-indigo-700' : 'text-slate-600 hover:bg-slate-50 hover:text-slate-900'}
      ${size === 'sm' ? 'whitespace-nowrap' : ''}
    `}
  >
    {icon}
    {label}
  </button>
);

const AuthScreen = ({ users, onLogin, onRegister }: { users: User[], onLogin: (u: User) => void, onRegister: (u: User) => void }) => {
  const [isLogin, setIsLogin] = useState(true);
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [confirm, setConfirm] = useState('');
  const [error, setError] = useState('');

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    setError('');

    if (isLogin) {
      const user = users.find(u => u.username === username);
      if (user && user.passwordHash === hashPassword(password)) {
        onLogin(user);
      } else {
        setError("Invalid credentials");
      }
    } else {
      // Register
      if (username.length < 3) return setError("Username too short");
      if (password.length < 3) return setError("Password too short");
      if (password !== confirm) return setError("Passwords do not match");
      if (users.find(u => u.username === username)) return setError("Username already exists");

      const newUser: User = { username, passwordHash: hashPassword(password), role: 'Client' };
      onRegister(newUser);
      // Auto login after register
      onLogin(newUser); 
    }
  };

  return (
    <div className="min-h-screen bg-slate-50 flex flex-col justify-center py-12 sm:px-6 lg:px-8">
      <div className="sm:mx-auto sm:w-full sm:max-w-md">
        <div className="w-12 h-12 bg-indigo-600 rounded-xl flex items-center justify-center mx-auto shadow-lg">
          <span className="text-white font-bold text-2xl">L</span>
        </div>
        <h2 className="mt-6 text-center text-3xl font-extrabold text-slate-900">
          {isLogin ? 'Sign in to LibraSys' : 'Create your account'}
        </h2>
      </div>

      <div className="mt-8 sm:mx-auto sm:w-full sm:max-w-md">
        <div className="bg-white py-8 px-4 shadow sm:rounded-lg sm:px-10">
          <form className="space-y-6" onSubmit={handleSubmit}>
            <Input label="Username" value={username} onChange={e => setUsername(e.target.value)} required />
            <Input label="Password" type="password" value={password} onChange={e => setPassword(e.target.value)} required />
            {!isLogin && (
              <Input label="Confirm Password" type="password" value={confirm} onChange={e => setConfirm(e.target.value)} required />
            )}
            
            {error && <div className="text-red-600 text-sm bg-red-50 p-2 rounded">{error}</div>}

            <Button type="submit" className="w-full">
              {isLogin ? 'Sign in' : 'Create Account'}
            </Button>
          </form>

          <div className="mt-6">
            <div className="relative">
              <div className="absolute inset-0 flex items-center"><div className="w-full border-t border-slate-300" /></div>
              <div className="relative flex justify-center text-sm">
                <span className="px-2 bg-white text-slate-500">Or</span>
              </div>
            </div>
            <div className="mt-6 text-center">
              <button onClick={() => setIsLogin(!isLogin)} className="text-indigo-600 hover:text-indigo-500 font-medium">
                {isLogin ? "Need an account? Register" : "Already have an account? Sign in"}
              </button>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

// --- Catalog View ---
const CatalogView = ({ media, userRole, onUpdate, onDelete, onAdd }: { 
  media: MediaItem[], 
  userRole: Role, 
  onUpdate: (m: MediaItem) => void, 
  onDelete: (id: number) => void,
  onAdd: (m: MediaItem) => void
}) => {
  const [search, setSearch] = useState('');
  const [isAddModalOpen, setAddModalOpen] = useState(false);
  
  // Filter
  const filtered = media.filter(m => m.titre.toLowerCase().includes(search.toLowerCase()));

  // Add Item State
  const [newItemType, setNewItemType] = useState<string>('Livre');
  const [newItemData, setNewItemData] = useState<Partial<MediaItem>>({});

  const handleAddItem = () => {
    const id = Math.max(...media.map(m => m.id), 0) + 1;
    const base = { id, titre: newItemData.titre || 'Untitled', dispo: true, type: newItemType as any };
    // Merge specific fields based on type (simplified for demo)
    onAdd({ ...base, ...newItemData } as MediaItem);
    setAddModalOpen(false);
    setNewItemData({});
  };

  return (
    <div className="space-y-6">
      <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
        <h1 className="text-2xl font-bold text-slate-900">Library Catalog</h1>
        <div className="flex gap-2 w-full sm:w-auto">
          <Input 
            placeholder="Search titles..." 
            value={search} 
            onChange={e => setSearch(e.target.value)} 
            className="w-full sm:w-64"
          />
          {(userRole === 'Admin' || userRole === 'SuperAdmin') && (
            <Button onClick={() => setAddModalOpen(true)}>+ Add Media</Button>
          )}
        </div>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
        {filtered.map(item => (
          <Card key={item.id} className="flex flex-col h-full">
            <div className="p-5 flex-1">
              <div className="flex justify-between items-start mb-2">
                <Badge color={item.type === 'Video' ? 'blue' : item.type === 'Audio' ? 'green' : 'gray'}>
                  {item.type}
                </Badge>
                <Badge color={item.dispo ? 'green' : 'red'}>
                  {item.dispo ? 'Available' : 'Borrowed'}
                </Badge>
              </div>
              <h3 className="text-lg font-bold text-slate-900 mb-1">{item.titre}</h3>
              
              {/* Dynamic Details based on Type */}
              <div className="text-sm text-slate-600 space-y-1 mt-3">
                {'auteur' in item && <p><span className="font-medium">Author:</span> {item.auteur}</p>}
                {'publicateur' in item && <p><span className="font-medium">Publisher:</span> {item.publicateur}</p>}
                {'nPage' in item && <p><span className="font-medium">Pages:</span> {item.nPage}</p>}
                {'duree' in item && <p><span className="font-medium">Duration:</span> {item.duree} min</p>}
                {'qualite' in item && <p><span className="font-medium">Quality:</span> {item.qualite}</p>}
                {'tailleMo' in item && <p><span className="font-medium">Size:</span> {item.tailleMo} MB</p>}
              </div>
            </div>
            
            <div className="p-4 bg-slate-50 border-t border-slate-100 flex justify-between items-center">
              <Button 
                variant={item.dispo ? 'primary' : 'secondary'} 
                size="sm"
                onClick={() => onUpdate({ ...item, dispo: !item.dispo })}
              >
                {item.dispo ? 'Borrow' : 'Return'}
              </Button>
              
              {(userRole === 'Admin' || userRole === 'SuperAdmin') && (
                <button 
                  onClick={() => onDelete(item.id)}
                  className="text-red-600 hover:text-red-800 text-sm font-medium"
                >
                  Delete
                </button>
              )}
            </div>
          </Card>
        ))}
      </div>

      {filtered.length === 0 && (
        <div className="text-center py-12 text-slate-500">
          No media found matching your search.
        </div>
      )}

      {/* Add Media Modal */}
      <Modal isOpen={isAddModalOpen} onClose={() => setAddModalOpen(false)} title="Add New Media">
        <div className="space-y-4">
          <Select 
            label="Type" 
            options={['Livre', 'Video', 'Audio', 'Ebook', 'AudioBook'].map(t => ({ value: t, label: t }))}
            value={newItemType}
            onChange={e => setNewItemType(e.target.value)}
          />
          <Input label="Title" value={newItemData.titre || ''} onChange={e => setNewItemData({...newItemData, titre: e.target.value})} />
          
          {/* Conditional Fields */}
          {(newItemType === 'Livre' || newItemType === 'Ebook' || newItemType === 'AudioBook') && (
             <Input label="Author" value={(newItemData as any).auteur || ''} onChange={e => setNewItemData({...newItemData, auteur: e.target.value})} />
          )}
           {(newItemType === 'Livre' || newItemType === 'Ebook' || newItemType === 'AudioBook') && (
             <Input label="Pages" type="number" value={(newItemData as any).nPage || 0} onChange={e => setNewItemData({...newItemData, nPage: parseInt(e.target.value)})} />
          )}
          {(newItemType === 'Video' || newItemType === 'Audio' || newItemType === 'AudioBook') && (
             <Input label="Duration (min)" type="number" value={(newItemData as any).duree || 0} onChange={e => setNewItemData({...newItemData, duree: parseInt(e.target.value)})} />
          )}
          
          <div className="pt-4 flex justify-end gap-2">
            <Button variant="secondary" onClick={() => setAddModalOpen(false)}>Cancel</Button>
            <Button onClick={handleAddItem}>Add Item</Button>
          </div>
        </div>
      </Modal>
    </div>
  );
};

// --- Stats View ---
const StatsView = ({ media }: { media: MediaItem[] }) => {
  const typeData = useMemo(() => {
    const counts: Record<string, number> = {};
    media.forEach(m => counts[m.type] = (counts[m.type] || 0) + 1);
    return Object.keys(counts).map(key => ({ name: key, value: counts[key] }));
  }, [media]);

  const availabilityData = useMemo(() => [
    { name: 'Available', value: media.filter(m => m.dispo).length },
    { name: 'Borrowed', value: media.filter(m => !m.dispo).length }
  ], [media]);

  const COLORS = ['#4f46e5', '#10b981', '#f59e0b', '#ef4444', '#8b5cf6'];

  return (
    <div className="space-y-6">
      <h1 className="text-2xl font-bold text-slate-900">Library Statistics</h1>
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        <Card className="p-6">
          <h3 className="text-lg font-medium text-slate-900 mb-4">Media Distribution</h3>
          <div className="h-64">
            <ResponsiveContainer width="100%" height="100%">
              <BarChart data={typeData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="name" />
                <YAxis />
                <Tooltip />
                <Bar dataKey="value" fill="#4f46e5" radius={[4, 4, 0, 0]} />
              </BarChart>
            </ResponsiveContainer>
          </div>
        </Card>
        
        <Card className="p-6">
          <h3 className="text-lg font-medium text-slate-900 mb-4">Availability Status</h3>
          <div className="h-64">
             <ResponsiveContainer width="100%" height="100%">
              <PieChart>
                <Pie data={availabilityData} cx="50%" cy="50%" innerRadius={60} outerRadius={80} paddingAngle={5} dataKey="value">
                  {availabilityData.map((entry, index) => (
                    <Cell key={`cell-${index}`} fill={entry.name === 'Available' ? '#10b981' : '#ef4444'} />
                  ))}
                </Pie>
                <Tooltip />
                <Legend />
              </PieChart>
            </ResponsiveContainer>
          </div>
        </Card>
      </div>
      
      <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
         <div className="bg-white p-6 rounded-lg shadow border border-slate-200">
           <p className="text-sm text-slate-500 font-medium">Total Items</p>
           <p className="text-3xl font-bold text-slate-900">{media.length}</p>
         </div>
         <div className="bg-white p-6 rounded-lg shadow border border-slate-200">
           <p className="text-sm text-slate-500 font-medium">Total Duration (Audio/Video)</p>
           <p className="text-3xl font-bold text-slate-900">
             {media.reduce((acc, curr) => acc + ((curr as any).duree || 0), 0)} <span className="text-sm font-normal text-slate-400">min</span>
           </p>
         </div>
          <div className="bg-white p-6 rounded-lg shadow border border-slate-200">
           <p className="text-sm text-slate-500 font-medium">Total Pages</p>
           <p className="text-3xl font-bold text-slate-900">
             {media.reduce((acc, curr) => acc + ((curr as any).nPage || 0), 0)}
           </p>
         </div>
      </div>
    </div>
  );
};

// --- User Management View ---
const UserManagementView = ({ users, currentUsername, onUpdateUser, onDeleteUser, onAddUser }: any) => {
  const [newUser, setNewUser] = useState({ username: '', password: '', role: 'Client' });

  const handleAdd = () => {
    if(!newUser.username || !newUser.password) return;
    onAddUser({ username: newUser.username, passwordHash: hashPassword(newUser.password), role: newUser.role });
    setNewUser({ username: '', password: '', role: 'Client' });
  };

  return (
    <div className="space-y-6">
      <h1 className="text-2xl font-bold text-slate-900">User Management</h1>
      
      <Card className="overflow-hidden">
        <table className="min-w-full divide-y divide-slate-200">
          <thead className="bg-slate-50">
            <tr>
              <th className="px-6 py-3 text-left text-xs font-medium text-slate-500 uppercase tracking-wider">Username</th>
              <th className="px-6 py-3 text-left text-xs font-medium text-slate-500 uppercase tracking-wider">Role</th>
              <th className="px-6 py-3 text-right text-xs font-medium text-slate-500 uppercase tracking-wider">Actions</th>
            </tr>
          </thead>
          <tbody className="bg-white divide-y divide-slate-200">
            {users.map((u: User) => (
              <tr key={u.username}>
                <td className="px-6 py-4 whitespace-nowrap text-sm font-medium text-slate-900">{u.username}</td>
                <td className="px-6 py-4 whitespace-nowrap text-sm text-slate-500">
                   <span className={`inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium ${u.role === 'SuperAdmin' ? 'bg-purple-100 text-purple-800' : u.role === 'Admin' ? 'bg-blue-100 text-blue-800' : 'bg-gray-100 text-gray-800'}`}>
                    {u.role}
                  </span>
                </td>
                <td className="px-6 py-4 whitespace-nowrap text-right text-sm font-medium">
                  {u.username !== currentUsername && (
                    <button onClick={() => onDeleteUser(u.username)} className="text-red-600 hover:text-red-900">Delete</button>
                  )}
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </Card>

      <Card className="p-6 max-w-xl">
        <h3 className="text-lg font-medium text-slate-900 mb-4">Add New User</h3>
        <div className="grid grid-cols-1 sm:grid-cols-2 gap-4 mb-4">
          <Input label="Username" value={newUser.username} onChange={e => setNewUser({...newUser, username: e.target.value})} />
          <Input label="Password" type="password" value={newUser.password} onChange={e => setNewUser({...newUser, password: e.target.value})} />
          <div className="sm:col-span-2">
            <Select 
              label="Role" 
              options={['Client', 'Admin', 'SuperAdmin'].map(r => ({ value: r, label: r }))}
              value={newUser.role}
              onChange={e => setNewUser({...newUser, role: e.target.value})}
            />
          </div>
        </div>
        <Button onClick={handleAdd} className="w-full sm:w-auto">Create User</Button>
      </Card>
    </div>
  );
};

// --- AI Librarian View ---
const AILibrarianView = ({ media }: { media: MediaItem[] }) => {
  const [input, setInput] = useState('');
  const [history, setHistory] = useState<ChatMessage[]>([
    { role: 'model', text: 'Hello! I am LibBot. How can I help you navigate the library today?', timestamp: Date.now() }
  ]);
  const [loading, setLoading] = useState(false);
  const bottomRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    bottomRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [history]);

  const handleSend = async () => {
    if (!input.trim() || loading) return;
    
    const userMsg: ChatMessage = { role: 'user', text: input, timestamp: Date.now() };
    setHistory(prev => [...prev, userMsg]);
    setInput('');
    setLoading(true);

    const responseText = await askLibrarian(input, media, history);
    
    setHistory(prev => [...prev, { role: 'model', text: responseText, timestamp: Date.now() }]);
    setLoading(false);
  };

  return (
    <div className="h-[calc(100vh-140px)] flex flex-col">
      <div className="flex-1 bg-white border border-slate-200 rounded-lg shadow-sm overflow-hidden flex flex-col">
        <div className="bg-indigo-600 p-4">
          <h2 className="text-white font-bold flex items-center gap-2">
            <svg className="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M8 10h.01M12 10h.01M16 10h.01M9 16H5a2 2 0 01-2-2V6a2 2 0 012-2h14a2 2 0 012 2v8a2 2 0 01-2 2h-5l-5 5v-5z" /></svg>
            AI Librarian Assistant
          </h2>
          <p className="text-indigo-200 text-sm">Ask about our books, authors, or get recommendations.</p>
        </div>
        
        <div className="flex-1 overflow-y-auto p-4 space-y-4 bg-slate-50">
          {history.map((msg, idx) => (
            <div key={idx} className={`flex ${msg.role === 'user' ? 'justify-end' : 'justify-start'}`}>
              <div className={`max-w-[80%] rounded-lg px-4 py-2 text-sm shadow-sm ${msg.role === 'user' ? 'bg-indigo-600 text-white' : 'bg-white text-slate-800 border border-slate-200'}`}>
                <p className="whitespace-pre-wrap">{msg.text}</p>
              </div>
            </div>
          ))}
          {loading && (
            <div className="flex justify-start">
              <div className="bg-white text-slate-800 border border-slate-200 rounded-lg px-4 py-2 text-sm shadow-sm animate-pulse">
                Thinking...
              </div>
            </div>
          )}
          <div ref={bottomRef} />
        </div>

        <div className="p-4 bg-white border-t border-slate-200">
          <div className="flex gap-2">
            <Input 
              placeholder="Ask for a book recommendation..." 
              value={input} 
              onChange={e => setInput(e.target.value)}
              onKeyDown={e => e.key === 'Enter' && handleSend()}
              disabled={loading}
              className="flex-1"
            />
            <Button onClick={handleSend} disabled={loading || !input.trim()}>
              Send
            </Button>
          </div>
          {!process.env.API_KEY && (
             <p className="text-xs text-red-500 mt-2">Note: AI features require an API_KEY in environment variables.</p>
          )}
        </div>
      </div>
    </div>
  );
};
