<script>
    import {initializeApp} from 'firebase/app';
    import {collection, getFirestore, getDocs} from 'firebase/firestore';

    const firebaseConfig = {
    apiKey: "AIzaSyCdPg9PiEZ96t1SCAeK_nemGXE0bnkfLl4",
    authDomain: "esp32firebase-d1cf9.firebaseapp.com",
    databaseURL: "https://esp32firebase-d1cf9-default-rtdb.firebaseio.com",
    projectId: "esp32firebase-d1cf9",
    storageBucket: "esp32firebase-d1cf9.firebasestorage.app",
    messagingSenderId: "655736330358",
    appId: "1:655736330358:web:83088abe6366182e63a757",
    measurementId: "G-XT75XXYYG4"
    };

    const app =  initializeApp(firebaseConfig);
    const db = getFirestore(app);
     
    async function queryCollecton() {
        const  collectionRef =  collection(db, 'perfis')
        const perfis = await getDocs(collectionRef)

        perfis.forEach(perfil => {
            console.log(perfil.data())
        });
    }

    queryCollecton();

</script>

<style>
    .menu { font-family: Arial, sans-serif; text-align: center; margin: 50px; display: flex; align-items: center; flex-direction: column;}
    h1 { color: #333; }
    p { font-size: 18px; }
    .led { margin: 20px; }
</style>
  
<div class="menu">
    <h1>Monitor de Ambiente</h1>
    <p><strong>Umidade:</strong>  varivavel aqui </p>
    <p><strong>Temperatura:</strong> varivavel aqui </p>

    <div class="led">
        <a href="/addPerfil"><button>Adicionar Perfil</button></a>
        <a href="/removePerfil"><button>Excluir Perfil</button></a>
      </div>
 </div>
